// Copyright Omkar Kulkarni
//Camera Occlusion code taken from Alfred Baudisch's github :- https://gist.github.com/alfredbaudisch/7978f1913e76640e2dc25b770ffa6ae1
//Link to Alfred Baudisch's blog used to create the camera occlusion effect to fade out actors in between camera and character :-
//https://alfredbaudisch.com/blog/gamedev/unreal-engine-ue/unreal-engine-actors-transparent-block-camera-occlusion-see-through/


#include "Player/AuraPlayerController.h"
#include "Interaction/EnemyInterface.h"
#include "Input/AuraInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"
#include "Components/SplineComponent.h"
#include "AuraGameplayTags.h"
#include "NavigationSystem.h"
#include "NavigationPath.h"
#include "GameFramework/Character.h"
#include "UI/Widget/DamageTextComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Containers/Set.h"

AAuraPlayerController::AAuraPlayerController()
{
    /* Camera Occlusion variables */

    CapsulePercentageForTrace = 1.0f;
    DebugLineTraces = true;
    IsOcclusionEnabled = true;

    /* Camera Occlusion variables */

    bReplicates = true;
    Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void AAuraPlayerController::BeginPlay()
{
    Super::BeginPlay();

    check(AuraContext);

    UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
    if(Subsystem)
    {
        Subsystem->AddMappingContext(AuraContext, 0);
    }

    bShowMouseCursor = true;
    DefaultMouseCursor = EMouseCursor::Default;

    FInputModeGameAndUI InputModeData;
    InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
    InputModeData.SetHideCursorDuringCapture(false);
    SetInputMode(InputModeData);

    if (IsValid(GetPawn()))
    {
        ActiveSpringArm = Cast<USpringArmComponent>(GetPawn()->GetComponentByClass(USpringArmComponent::StaticClass()));
        ActiveCamera = Cast<UCameraComponent>(GetPawn()->GetComponentByClass(UCameraComponent::StaticClass()));
        ActiveCapsuleComponent = Cast<UCapsuleComponent>(GetPawn()->GetComponentByClass(UCapsuleComponent::StaticClass()));
    }

}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    CursorTrace();
    AutoRun();
}

void AAuraPlayerController::ShowDamageNumber_Implementation(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit)
{
    if(IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController())
    {
        UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
        DamageText->RegisterComponent();
        DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
        DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
        DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
    }
}

void AAuraPlayerController::AutoRun()
{
    if(!bAutoRunning) return;
    if(APawn* ControlledPawn = GetPawn())
    {
        const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
        const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
        ControlledPawn->AddMovementInput(Direction);

        const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
        if(DistanceToDestination <= AutoRunAcceptanceRadius)
        {
            bAutoRunning = false;
        }
    }
}

void AAuraPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

    AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
    AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &AAuraPlayerController::ShiftPressed);
    AuraInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &AAuraPlayerController::ShiftReleased);
    AuraInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void AAuraPlayerController::Move(const FInputActionValue &InputActionValue)
{
    const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
    const FRotator YawRotation(0.f, GetControlRotation().Yaw, 0.f);

    const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    if (APawn* ControlledPawn = GetPawn<APawn>())
    {
        ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
        ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
    }
}

void AAuraPlayerController::CursorTrace()
{
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
    if(!CursorHit.bBlockingHit) return;

    LastActor = ThisActor;
    ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

    if(LastActor != ThisActor)
    {
        if(LastActor) LastActor->UnHighlightActor();
        if(ThisActor) ThisActor->HighlightActor();
    }
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    if(InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
    {
        bTargeting = ThisActor ? true : false;
        bAutoRunning = false;
    }
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if(!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
    {
        if(GetASC()) GetASC()->AbilityInputTagReleased(InputTag);
        return;
    }

    if(GetASC()) GetASC()->AbilityInputTagReleased(InputTag);

    if(!bTargeting && !bShiftKeyDown)
    {
        const APawn* ControlledPawn = GetPawn();
        if(FollowTime <= ShortPressThreshold && ControlledPawn)
        {
            if(UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
            {
                Spline->ClearSplinePoints();
                for(const FVector& PointLoc : NavPath->PathPoints)
                {
                    Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
                }

                if (NavPath->PathPoints.Num() > 0)
                {
                    CachedDestination = NavPath->PathPoints[NavPath->PathPoints.Num() - 1];
                    bAutoRunning = true;
                }
            }
        }
        FollowTime = 0.f;
        bTargeting = false;
    }
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if(!InputTag.MatchesTagExact(FAuraGameplayTags::Get().InputTag_LMB))
    {
        if(GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
        return;
    }

    if(bTargeting || bShiftKeyDown)
    {
        if(GetASC()) GetASC()->AbilityInputTagHeld(InputTag);
    }
    else
    {
        FollowTime += GetWorld()->GetDeltaSeconds();
        if(CursorHit.bBlockingHit) CachedDestination = CursorHit.ImpactPoint;

        if(APawn* ControlledPawn = GetPawn())
        {
            const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
            ControlledPawn->AddMovementInput(WorldDirection);
        }
    }
}

UAuraAbilitySystemComponent* AAuraPlayerController::GetASC()
{
    if(AuraAbilitySystemComponent == nullptr)
    {
        UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>());
        AuraAbilitySystemComponent = Cast<UAuraAbilitySystemComponent>(ASC);
    }

    return AuraAbilitySystemComponent;
}

void AAuraPlayerController::SyncOccludedActors()
{
    if (!ShouldCheckCameraOcclusion()) return;

    // Camera is currently colliding, show all current occluded actors
    // and do not perform further occlusion
    if (ActiveSpringArm->bDoCollisionTest)
    {
        ForceShowOccludedActors();
        return;
    }

    FVector Start = ActiveCamera->GetComponentLocation();
    FVector End = GetPawn()->GetActorLocation();

    TArray<TEnumAsByte<EObjectTypeQuery>> CollisionObjectTypes;
    CollisionObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldStatic));

    TArray<AActor*> ActorsToIgnore; // TODO: Add configuration to ignore actor types
    TArray<FHitResult> OutHits;

    auto ShouldDebug = DebugLineTraces ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

    bool bGotHits = UKismetSystemLibrary::CapsuleTraceMultiForObjects(
        GetWorld(), Start, End, ActiveCapsuleComponent->GetScaledCapsuleRadius() * CapsulePercentageForTrace,
        ActiveCapsuleComponent->GetScaledCapsuleHalfHeight() * CapsulePercentageForTrace, CollisionObjectTypes, true,
        ActorsToIgnore,
        ShouldDebug,
        OutHits, true);

    if (bGotHits)
    {
        // The list of actors hit by the line trace, that means that they are occluded from view
        TSet<const AActor*> ActorsJustOccluded;

        // Hide actors that are occluded by the camera
        for (FHitResult Hit : OutHits)
        {
            const AActor* HitActor = Cast<AActor>(Hit.GetActor());
            HideOccludedActor(HitActor);
            ActorsJustOccluded.Add(HitActor);
        }

        // Show actors that are currently hidden but that are not occluded by the camera anymore 
        for (auto& Elem : OccludedActors)
        {
            if (!ActorsJustOccluded.Contains(Elem.Value.Actor) && Elem.Value.IsOccluded)
            {
                ShowOccludedActor(Elem.Value);

                if (DebugLineTraces)
                {
                    UE_LOG(LogTemp, Warning,
                        TEXT("Actor %s was occluded, but it's not occluded anymore with the new hits."), *Elem.Value.Actor->GetName());
                }
            }
        }
    }
    else
    {
        ForceShowOccludedActors();
    }
}

bool AAuraPlayerController::HideOccludedActor(const AActor* Actor)
{
    FCameraOccludedActor* ExistingOccludedActor = OccludedActors.Find(Actor);

    if (ExistingOccludedActor && ExistingOccludedActor->IsOccluded)
    {
        if (DebugLineTraces) UE_LOG(LogTemp, Warning, TEXT("Actor %s was already occluded. Ignoring."),
            *Actor->GetName());
        return false;
    }

    if (ExistingOccludedActor && IsValid(ExistingOccludedActor->Actor))
    {
        ExistingOccludedActor->IsOccluded = true;
        OnHideOccludedActor(*ExistingOccludedActor);

        if (DebugLineTraces) UE_LOG(LogTemp, Warning, TEXT("Actor %s exists, but was not occluded. Occluding it now."), *Actor->GetName());
    }
    else
    {
        UStaticMeshComponent* StaticMesh = Cast<UStaticMeshComponent>(
            Actor->GetComponentByClass(UStaticMeshComponent::StaticClass()));

        FCameraOccludedActor OccludedActor;
        OccludedActor.Actor = Actor;
        OccludedActor.StaticMesh = StaticMesh;
        OccludedActor.Materials = StaticMesh->GetMaterials();
        OccludedActor.IsOccluded = true;
        OccludedActors.Add(Actor, OccludedActor);
        OnHideOccludedActor(OccludedActor);

        if (DebugLineTraces) UE_LOG(LogTemp, Warning, TEXT("Actor %s does not exist, creating and occluding it now."), *Actor->GetName());
    }

    return true;
}


void AAuraPlayerController::ForceShowOccludedActors()
{
    for (auto& Elem : OccludedActors)
    {
        if (Elem.Value.IsOccluded)
        {
            ShowOccludedActor(Elem.Value);

            if (DebugLineTraces) UE_LOG(LogTemp, Warning, TEXT("Actor %s was occluded, force to show again."), *Elem.Value.Actor->GetName());
        }
    }
}

void AAuraPlayerController::ShowOccludedActor(FCameraOccludedActor& OccludedActor)
{
    if (!IsValid(OccludedActor.Actor))
    {
        OccludedActors.Remove(OccludedActor.Actor);
    }

    OccludedActor.IsOccluded = false;
    OnShowOccludedActor(OccludedActor);
}

bool AAuraPlayerController::OnShowOccludedActor(const FCameraOccludedActor& OccludedActor) const
{
    for (int matIdx = 0; matIdx < OccludedActor.Materials.Num(); ++matIdx)
    {
        OccludedActor.StaticMesh->SetMaterial(matIdx, OccludedActor.Materials[matIdx]);
    }

    return true;
}

bool AAuraPlayerController::OnHideOccludedActor(const FCameraOccludedActor& OccludedActor) const
{
    for (int i = 0; i < OccludedActor.StaticMesh->GetNumMaterials(); ++i)
    {
        OccludedActor.StaticMesh->SetMaterial(i, FadeMaterial);
    }

    return true;
}

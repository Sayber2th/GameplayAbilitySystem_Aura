// Copyright Omkar Kulkarni


#include "Player/AuraPlayerController.h"
#include "Interaction/EnemyInterface.h"
#include "Input/AuraInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

AAuraPlayerController::AAuraPlayerController()
{
    bReplicates = true;
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
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
    Super::PlayerTick(DeltaTime);

    CursorTrace();
}

void AAuraPlayerController::SetupInputComponent()
{
    Super::SetupInputComponent();

    UAuraInputComponent* AuraInputComponent = CastChecked<UAuraInputComponent>(InputComponent);

    AuraInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
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
    FHitResult CursorHit;
    GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, false, CursorHit);
    if(!CursorHit.bBlockingHit) return;

    LastActor = ThisActor;
    ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

    /**
     * Line trace from cursor. There are several scenarios:
     * A. LastActor is null && ThisActor is null
     *      -Do nothing
     * B. LastActor is null && ThisActor is valid
     *      -Highlight ThisActor
     * C. LastActor is valid && ThisActor is null
     *      -Unhighlight the LastActor
     * D. Both actors are valid; LastActor != ThisActor
     *      -Unhighlight LastActor & Highlight ThisActor
     * E. Both actors are valid; LastActor == ThisActor
     *      -Do nothing
    */

   if(LastActor == nullptr)
   {
        if(ThisActor != nullptr)
        {
            //Case B
            ThisActor->HighlightActor();
        }
        else
        {
            //Case A - do nothing
        }
   }
   else //LastActor is valid
   {
        if(ThisActor == nullptr)
        {
            //Case C
            LastActor->UnHighlightActor();
        }
        else //Both actors are valid
        {
            if(LastActor != ThisActor)
            {
                //Case D
                LastActor->UnHighlightActor();
                ThisActor->HighlightActor();
            }
            else
            {
                //Case E - Do nothing
            }
        }
   }
}

void AAuraPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
    //GEngine->AddOnScreenDebugMessage(1, 3.f, FColor::Red, *InputTag.ToString());
}

void AAuraPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
    if(GetASC() == nullptr) return;
    GetASC()->AbilityInputTagReleased(InputTag);
}

void AAuraPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
    if(GetASC() == nullptr) return;
    GetASC()->AbilityInputTagHeld(InputTag);
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

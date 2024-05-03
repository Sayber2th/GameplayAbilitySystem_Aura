// Copyright Omkar Kulkarni
//Camera Occlusion code taken from Alfred Baudisch's github :- https://gist.github.com/alfredbaudisch/7978f1913e76640e2dc25b770ffa6ae1
//Link to Alfred Baudisch's blog used to create the camera occlusion effect to fade out actors in between camera and character :-
//https://alfredbaudisch.com/blog/gamedev/unreal-engine-ue/unreal-engine-actors-transparent-block-camera-occlusion-see-through/

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "GameplayTagContainer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "AuraPlayerController.generated.h"


class UDamageTextComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class IEnemyInterface;
class UAuraInputConfig;
class UAuraAbilitySystemComponent;
class USplineComponent;

/* Camera Occlusion */

USTRUCT(BlueprintType)
struct FCameraOccludedActor
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	const AActor* Actor;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* StaticMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<UMaterialInterface*> Materials;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	bool IsOccluded = false;
};

/* Camera Occlusion */

/**
 * 
 */
UCLASS()
class AURA_API AAuraPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AAuraPlayerController();
	virtual void PlayerTick(float DeltaTime) override;

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(float DamageAmount, ACharacter* TargetCharacter, bool bBlockedHit, bool bCriticalHit);

protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	/* Camera Occlusion */

	/** How much of the Pawn capsule Radius and Height
	* should be used for the Line Trace before considering an Actor occluded?
	* Values too low may make the camera clip through walls.
	*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Occlusion|Occlusion",
		meta = (ClampMin = "0.1", ClampMax = "10.0"))
	float CapsulePercentageForTrace;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Occlusion|Materials")
	UMaterialInterface* FadeMaterial;

	UPROPERTY(BlueprintReadWrite, Category = "Camera Occlusion|Components")
	class USpringArmComponent* ActiveSpringArm;

	UPROPERTY(BlueprintReadWrite, Category = "Camera Occlusion|Components")
	class UCameraComponent* ActiveCamera;

	UPROPERTY(BlueprintReadWrite, Category = "Camera Occlusion|Components")
	class UCapsuleComponent* ActiveCapsuleComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Occlusion")
	bool IsOcclusionEnabled;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Occlusion|Occlusion")
	bool DebugLineTraces;

	/* Camera Occlusion */

private:
	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputMappingContext> AuraContext;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = Input)
	TObjectPtr<UInputAction> ShiftAction;

	void Move(const FInputActionValue& InputActionValue);

	void ShiftPressed() { bShiftKeyDown = true; };
	void ShiftReleased() { bShiftKeyDown = false; };
	bool bShiftKeyDown = false;

	void CursorTrace();
	IEnemyInterface* ThisActor;
	IEnemyInterface* LastActor;
	FHitResult CursorHit;

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<UAuraInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<UAuraAbilitySystemComponent> AuraAbilitySystemComponent;

	UAuraAbilitySystemComponent* GetASC();

	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	bool bTargeting;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	void AutoRun();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	/* Camera Occlusion */

	TMap<const AActor*, FCameraOccludedActor> OccludedActors;

	bool HideOccludedActor(const AActor* Actor);
	bool OnHideOccludedActor(const FCameraOccludedActor& OccludedActor) const;
	void ShowOccludedActor(FCameraOccludedActor& OccludedActor);
	bool OnShowOccludedActor(const FCameraOccludedActor& OccludedActor) const;
	void ForceShowOccludedActors();

	__forceinline bool ShouldCheckCameraOcclusion() const
	{
		return IsOcclusionEnabled && FadeMaterial && ActiveCamera && ActiveCapsuleComponent;
	}

	/* Camera Occlusion */

public:

	/* Camera Occlusion */

	UFUNCTION(BlueprintCallable)
	void SyncOccludedActors();
	
	/* Camera Occlusion */

};

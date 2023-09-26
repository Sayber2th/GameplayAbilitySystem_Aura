// Copyright Omkar Kulkarni

#pragma once

#include "CoreMinimal.h"
#include "Character/AuraBaseCharacter.h"
#include "Interaction/EnemyInterface.h"
#include "AuraEnemy.generated.h"

/**
 * 
 */
UCLASS()
class AURA_API AAuraEnemy : public AAuraBaseCharacter, public IEnemyInterface
{
	GENERATED_BODY()
	
public:
	AAuraEnemy();

	/** Enemy Interface */
	virtual void HighlightActor() override;
	virtual void UnHighlightActor() override;
	/** end Enemy Interface*/

protected:
	virtual void BeginPlay() override;
	virtual void InitActorAbilityInfo();
};

// Copyright Omkar Kulkarni


#include "AbilitySystem/Abilities/AuraProjectileSpell.h"
#include "Actor/AuraProjectile.h"
#include "Interaction/CombatInterface.h"

void UAuraProjectileSpell::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo *ActorInfo, 
    const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData *TriggerEventData)
{
    Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

}

void UAuraProjectileSpell::SpawnProjectile(const FVector& ProjectileTargetLocation)
{
    const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
    if(!bIsServer) return;

    ICombatInterface* CombatInterface = Cast<ICombatInterface>(GetAvatarActorFromActorInfo());
    if(CombatInterface)
    {
        const FVector SocketLocation = CombatInterface->GetCombatSocketLocation();
        FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
        Rotation.Pitch = 0.f;
        
        FTransform SpawnTransform;
        SpawnTransform.SetLocation(SocketLocation);
        SpawnTransform.SetRotation(Rotation.Quaternion());

        AAuraProjectile* Projectile = GetWorld()->SpawnActorDeferred<AAuraProjectile>(
            ProjectileClass, 
            SpawnTransform, 
            GetOwningActorFromActorInfo(), 
            Cast<APawn>(GetOwningActorFromActorInfo()), 
            ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

        /* TO DO: Give the projectile a Gameplay Effect Spec for causing damage. */

        Projectile->FinishSpawning(SpawnTransform);
    }
}

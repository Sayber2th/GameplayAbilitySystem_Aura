// Copyright Omkar Kulkarni


#include "Character/AuraBaseCharacter.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystem/AuraAbilitySystemComponent.h"

AAuraBaseCharacter::AAuraBaseCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

UAbilitySystemComponent* AAuraBaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void AAuraBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
}

FVector AAuraBaseCharacter::GetCombatSocketLocation()
{
	check(Weapon);
	return Weapon->GetSocketLocation(WeaponTipSocketName);
}

void AAuraBaseCharacter::InitAbilityActorInfo()
{

}

void AAuraBaseCharacter::ApplyEffectToSelf(TSubclassOf<UGameplayEffect> GameplayEffectClass, float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(GameplayEffectClass);
	FGameplayEffectContextHandle ContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	ContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(GameplayEffectClass, Level, ContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void AAuraBaseCharacter::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1);
	ApplyEffectToSelf(DefaultVitalAttributes, 1);
}

void AAuraBaseCharacter::AddCharacterAbilities()
{
	UAuraAbilitySystemComponent* AuraASC = CastChecked<UAuraAbilitySystemComponent>(AbilitySystemComponent);
	if(!HasAuthority()) return;

	AuraASC->AddCharacterAbilities(StartupAbilities);
}

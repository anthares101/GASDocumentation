// Copyright 2020 Dan Kestranek.


#include "Characters/Heroes/Abilities/GDGA_BulletRain.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "Camera/CameraComponent.h"
#include "Characters/Heroes/GDHeroCharacter.h"
#include "Player/GDPlayerState.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGDGA_BulletRain::UGDGA_BulletRain()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag Ability8Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Ability8"));
	AbilityTags.AddTag(Ability8Tag);
	ActivationOwnedTags.AddTag(Ability8Tag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));

	Range = 1000.0f;
	Damage = 12.0f;
}

void UGDGA_BulletRain::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	AGDHeroCharacter* Hero = Cast<AGDHeroCharacter>(GetAvatarActorFromActorInfo());
	AGDPlayerState* PD = Cast <AGDPlayerState>(GetOwningActorFromActorInfo());
	if (!Hero || !PD)
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	FGameplayAbilityTargetingLocationInfo targetingLocationInfo = MakeTargetLocationInfoFromOwnerActor();
	FVector newLocation = Hero->GetActorForwardVector();
	newLocation = newLocation * 200 + targetingLocationInfo.LiteralTransform.GetLocation();
	targetingLocationInfo.LiteralTransform.SetLocation(newLocation);

	//Spawn the indicator
	TargetActor = GetWorld()->SpawnActorDeferred<AGameplayAbilityTargetActor_Trace>(TargetActorClass, targetingLocationInfo.LiteralTransform, GetOwningActorFromActorInfo(),
																				Hero, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	TargetActor->MaxRange = Range;
	TargetActor->StartLocation = targetingLocationInfo;
	TargetActor->FinishSpawning(targetingLocationInfo.LiteralTransform);

	UAbilityTask_WaitTargetData* Task = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActor);
	Task->ValidData.AddDynamic(this, &UGDGA_BulletRain::ValidData);
	Task->Cancelled.AddDynamic(this, &UGDGA_BulletRain::Cancelled);
	
	//Show confirmation message
	PD->ShowAbilityConfirmCancelText(true);

	// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	Task->ReadyForActivation();
}

void UGDGA_BulletRain::DeleteText() {
	AGDPlayerState* PD = Cast <AGDPlayerState>(GetOwningActorFromActorInfo());
	if (PD)
	{
		PD->ShowAbilityConfirmCancelText(false);
	}
}

void UGDGA_BulletRain::Cancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("CANCELLED"));
	DeleteText();

	TargetActor->Destroy();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGDGA_BulletRain::ValidData(const FGameplayAbilityTargetDataHandle& Data)
{
	UE_LOG(LogTemp, Warning, TEXT("VALID"));
	DeleteText();

	TargetActor->Destroy();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

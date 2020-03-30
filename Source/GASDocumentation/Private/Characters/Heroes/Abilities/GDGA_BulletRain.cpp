// Copyright 2020 Dan Kestranek.


#include "Characters/Heroes/Abilities/GDGA_BulletRain.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitTargetData.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbilityTargetActor.h"
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

	//Move cameraBoom up to help targeting
	FVector cameraBoomLoc = Hero->GetStartingCameraBoomLocation();
	cameraBoomLoc.Z = cameraBoomLoc.Z + 100;

	Hero->GetCameraBoom()->SetRelativeLocation(cameraBoomLoc);

	//Start a task that will wait for the user confirmation of the ability target
	Task = UAbilityTask_WaitTargetData::WaitTargetDataUsingActor(this, NAME_None, EGameplayTargetingConfirmation::UserConfirmed, TargetActor);
	Task->ValidData.AddDynamic(this, &UGDGA_BulletRain::ValidData);
	Task->Cancelled.AddDynamic(this, &UGDGA_BulletRain::Cancelled);
	
	//Show confirmation message
	PD->ShowAbilityConfirmCancelText(true);

	// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	Task->ReadyForActivation();
}

void UGDGA_BulletRain::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AGDHeroCharacter* Hero = Cast<AGDHeroCharacter>(GetAvatarActorFromActorInfo());
	AGDPlayerState* PD = Cast <AGDPlayerState>(GetOwningActorFromActorInfo());
	if (Hero && PD)
	{
		PD->ShowAbilityConfirmCancelText(false);
		Hero->GetCameraBoom()->SetRelativeLocation(Hero->GetStartingCameraBoomLocation());
	}

	TargetActor->Destroy();
}

void UGDGA_BulletRain::Cancelled(const FGameplayAbilityTargetDataHandle& Data)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGDGA_BulletRain::ValidData(const FGameplayAbilityTargetDataHandle& Data)
{
	if (!CommitAbility(GetCurrentAbilitySpecHandle(), GetCurrentActorInfo(), GetCurrentActivationInfo()))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	// Only spawn projectiles on the Server.
	if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority)
	{
		AGDHeroCharacter* Hero = Cast<AGDHeroCharacter>(GetAvatarActorFromActorInfo());
		if (!Hero)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		// Pass the damage to the Damage Execution Calculation through a SetByCaller value on the GameplayEffectSpec
		FGameplayEffectSpecHandle DamageEffectSpecHandle = MakeOutgoingGameplayEffectSpec(DamageGameplayEffect, GetAbilityLevel());
		DamageEffectSpecHandle.Data.Get()->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag(FName("Data.Damage")), Damage);

		// Calculate proyectiles tranformation
		FVector Start = Data.Get(0)->GetHitResult()->Location;
		Start.Z = Start.Z + 1000;

		FVector End = Data.Get(0)->GetHitResult()->ImpactPoint;
		FRotator Rotation = UKismetMathLibrary::FindLookAtRotation(Start, End);

		FTransform proyectileTransform = FTransform();
		proyectileTransform.SetLocation(Start);
		proyectileTransform.SetRotation(Rotation.Quaternion());
		proyectileTransform.SetScale3D(FVector(4.0f));

		//Spawn Proyectiles
		AGDProjectile* Projectile = GetWorld()->SpawnActorDeferred<AGDProjectile>(ProjectileClass, proyectileTransform, GetOwningActorFromActorInfo(),
																				  Hero, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
		Projectile->DamageEffectSpecHandle = DamageEffectSpecHandle;
		Projectile->Range = 1000;
		Projectile->FinishSpawning(proyectileTransform);

	}

	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

// Copyright 2020 Dan Kestranek.


#include "Characters/Heroes/Abilities/GDGA_Flash.h"
#include "AbilitySystemComponent.h"
#include "Camera/CameraComponent.h"
#include "Characters/Heroes/GDHeroCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/KismetMathLibrary.h"

UGDGA_Flash::UGDGA_Flash()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;

	FGameplayTag Ability6Tag = FGameplayTag::RequestGameplayTag(FName("Ability.Skill.Ability6"));
	AbilityTags.AddTag(Ability6Tag);
	ActivationOwnedTags.AddTag(Ability6Tag);

	ActivationBlockedTags.AddTag(FGameplayTag::RequestGameplayTag(FName("Ability.Skill")));

	Range = 10.0f;
}

void UGDGA_Flash::ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}

	UAnimMontage* MontageToPlay = FlashMontage;

	// Play fire montage and wait for event telling us to spawn the projectile
	UGDAT_PlayMontageAndWaitForEvent* Task = UGDAT_PlayMontageAndWaitForEvent::PlayMontageAndWaitForEvent(this, NAME_None, MontageToPlay, FGameplayTagContainer(), 1.0f, NAME_None, true, 1.0f);
	Task->OnBlendOut.AddDynamic(this, &UGDGA_Flash::OnCompleted);
	Task->OnCompleted.AddDynamic(this, &UGDGA_Flash::OnCompleted);
	Task->OnInterrupted.AddDynamic(this, &UGDGA_Flash::OnCancelled);
	Task->OnCancelled.AddDynamic(this, &UGDGA_Flash::OnCancelled);
	Task->EventReceived.AddDynamic(this, &UGDGA_Flash::EventReceived);
	// ReadyForActivation() is how you activate the AbilityTask in C++. Blueprint has magic from K2Node_LatentGameplayTaskCall that will automatically call ReadyForActivation().
	Task->ReadyForActivation();
}

void UGDGA_Flash::OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGDGA_Flash::OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData)
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGDGA_Flash::EventReceived(FGameplayTag EventTag, FGameplayEventData EventData)
{
	// Montage told us to end the ability before the montage finished playing.
	// Montage was set to continue playing animation even after ability ends so this is okay.
	if (EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.EndAbility")))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		return;
	}

	//The ability is executed on the server
	if (GetOwningActorFromActorInfo()->GetLocalRole() == ROLE_Authority && EventTag == FGameplayTag::RequestGameplayTag(FName("Event.Montage.Tp")))
	{
		AGDHeroCharacter* Hero = Cast<AGDHeroCharacter>(GetAvatarActorFromActorInfo());
		if (!Hero)
		{
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
		}

		Hero->SetActorLocation(Hero->GetActorForwardVector() * Range + Hero->GetActorLocation());
	}
	
}


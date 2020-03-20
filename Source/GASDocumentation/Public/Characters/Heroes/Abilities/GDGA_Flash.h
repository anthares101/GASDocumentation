// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/GDGameplayAbility.h"
#include "Characters/Abilities/AbilityTasks/GDAT_PlayMontageAndWaitForEvent.h"
#include "GDGA_Flash.generated.h"

/**
 * 
 */
UCLASS()
class GASDOCUMENTATION_API UGDGA_Flash : public UGDGameplayAbility
{
	GENERATED_BODY()
	
	public:
		UGDGA_Flash();

		UPROPERTY(BlueprintReadOnly, EditAnywhere)
			UAnimMontage* FlashMontage;

		/** Actually activate ability, do not call this directly. We'll call it from APAHeroCharacter::ActivateAbilitiesWithTags(). */
		virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	protected:
		UPROPERTY(BlueprintReadOnly, EditAnywhere)
			float Range;

		UFUNCTION()
			void OnCancelled(FGameplayTag EventTag, FGameplayEventData EventData);

		UFUNCTION()
			void OnCompleted(FGameplayTag EventTag, FGameplayEventData EventData);

		UFUNCTION()
			void EventReceived(FGameplayTag EventTag, FGameplayEventData EventData);
};

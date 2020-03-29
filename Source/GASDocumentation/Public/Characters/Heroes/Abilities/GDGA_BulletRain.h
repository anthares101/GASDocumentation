// Copyright 2020 Dan Kestranek.

#pragma once

#include "CoreMinimal.h"
#include "Characters/Abilities/GDGameplayAbility.h"
#include "Abilities/GameplayAbilityTargetActor_Trace.h"
#include "Characters/GDProjectile.h"
#include "GDGA_BulletRain.generated.h"

/**
 * 
 */
UCLASS()
class GASDOCUMENTATION_API UGDGA_BulletRain : public UGDGameplayAbility
{
	GENERATED_BODY()

public:
	UGDGA_BulletRain();

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		TSubclassOf<AGDProjectile> ProjectileClass;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		TSubclassOf<UGameplayEffect> DamageGameplayEffect;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		TSubclassOf<AGameplayAbilityTargetActor_Trace> TargetActorClass;

	/** Actually activate ability, do not call this directly. We'll call it from APAHeroCharacter::ActivateAbilitiesWithTags(). */
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

protected:
	//Targetting zone
	AGameplayAbilityTargetActor_Trace* TargetActor;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		float Range;

	UPROPERTY(BlueprintReadOnly, EditAnywhere)
		float Damage;

	//Destroy the confirm/cancel text
	void DeleteText();

	UFUNCTION()
		void ValidData(const FGameplayAbilityTargetDataHandle& Data);

	UFUNCTION()
		void Cancelled(const FGameplayAbilityTargetDataHandle& Data);
	
};

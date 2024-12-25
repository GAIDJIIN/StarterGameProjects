//Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootSoundComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(FootSoundComp, Log, All);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FOOTCOMPONENT_API UFootSoundComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UFootSoundComponent();

protected:
	virtual void BeginPlay() override;

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// CPP
	// Functions
	// Setter
	void SetCheckFootSoundTimer(const bool bIsCheck);

	// Getter
	const float GetOwnerXYSpeed() const;
	TSoftObjectPtr<USoundBase> GetFootSoundbySurface(const TEnumAsByte<EPhysicalSurface> Surface) const;
	const float GetFootSoundVolumeMultiplier() const;
	const float GetFootSoundPlayTime() const;
	 
private:

	// Properties
	// Blueprints
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup", meta=(ClampMin="0.001", AllowPrivateAccess))
		float CheckSpeedFreq = 0.25f;
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Sounds", meta=(AllowPrivateAccess))
		TMap<TEnumAsByte<EPhysicalSurface>, TSoftObjectPtr<USoundBase>> FootSoundBySurface;
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Curve", meta=(AllowPrivateAccess))
		TObjectPtr<UCurveFloat> CurveFootSoundPlayTime = nullptr;
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Curve", meta=(AllowPrivateAccess))
		TObjectPtr<UCurveFloat> CurveFootSoundVolumeMultiplier = nullptr;
	UPROPERTY(EditAnywhere, Category="Debug", meta=(AllowPrivateAccess))
		bool bShowDebug = false;
	
	// Service
	TWeakObjectPtr<ACharacter> OwnerCharRef;
	bool bIsPlayFootSoundNow = false;
	float LastSoundPlayTime = 0.f;
	float LastSoundVolumeMultiplier = 0.f;

	// Sound Info
	TSoftObjectPtr<USoundBase> CurrentFootSound = nullptr;
	FVector CurrentFootSoundLocation;
	
	// Timer Handle
	FTimerHandle CheckFootSoundTimerHandle;
	FTimerHandle PlayFootSoundTimerHandle;
	
	// Functions
	// Main Logic
	void CheckSpeedForFootSound();
	
	void TryPlayFootSound();
	void PlayFootSound();
	
	void SetFootSoundTimer();
	void TrySetFootSoundTimer();

	void TryLoadAndPlayFootSound();
	
	// Debug
	void ShowDebug();
};

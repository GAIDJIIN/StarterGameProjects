//Foot Sound Component. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "FootSoundComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogFootSoundComp, Log, All);

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
	// Check freq speed for play foot sounds
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup", meta=(ClampMin="0.001", AllowPrivateAccess))
		float CheckSpeedFreq = 0.25f;
	// Foot sounds by surface
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Sounds", meta=(AllowPrivateAccess))
		TMap<TEnumAsByte<EPhysicalSurface>, TSoftObjectPtr<USoundBase>> FootSoundBySurface;
	// Curve of foot sound play time (alpha time between two foot sounds)
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Curve", meta=(AllowPrivateAccess))
		TObjectPtr<UCurveFloat> CurveFootSoundPlayTime = nullptr;
	// Curve of foot sound volume multiplayer
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Curve", meta=(AllowPrivateAccess))
		TObjectPtr<UCurveFloat> CurveFootSoundVolumeMultiplier = nullptr;
	// Curve of foot noise loudness by speed
	UPROPERTY(EditAnywhere, Category="Foot Sound Setup|Curve", meta=(AllowPrivateAccess))
		TObjectPtr<UCurveFloat> CurveFootNoiseLoudnessMultiplayer = nullptr;
	UPROPERTY(EditAnywhere, Category="Debug", meta=(AllowPrivateAccess))
		bool bShowDebug = false;
	
	// Service
	TWeakObjectPtr<ACharacter> OwnerCharRef;
	TWeakObjectPtr<UPawnNoiseEmitterComponent> FootNoiseEmitter;
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
	
	void SetFootSoundTimer();
	void TrySetFootSoundTimer();

	void TryLoadAndPlayFootSound();

	// Getter

	// Get foot noise loudness by sound volume and speed
	FORCEINLINE float GetNoiseLoudnessBySpeed() const; 
	
	// RPC Methods
	
	UFUNCTION(Server, Unreliable, WithValidation, Category = "Foot Sound")
		void Server_PlayFootSound();
	// Play foot sound
	UFUNCTION(NetMulticast, Unreliable, Category = "Foot Sound")
		void NetMulticast_PlayFootSound();

	// Debug
	void ShowDebug();
};

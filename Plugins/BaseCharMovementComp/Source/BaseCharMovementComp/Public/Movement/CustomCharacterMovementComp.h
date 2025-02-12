// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComp.generated.h"


struct FActorComponentTickFunction;

UCLASS()
class BASECHARMOVEMENTCOMP_API UCustomCharacterMovementComp : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	// Override parents functions
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Deactivate() override;
	virtual void BeginPlay() override;
	virtual float GetMaxSpeed() const override;

	// Replicate properties
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	#if WITH_EDITOR
		virtual auto PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) -> void override;
	#endif
	
	// Reset Movement Comp
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_ResetMovementComp();
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="MovementInfo")
		void NetMulticast_ResetMovementComp();

	// Setup Movement Comp
	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_SetupMovementComp();
	UFUNCTION(NetMulticast, Reliable, BlueprintCallable, Category="MovementInfo")
		void NetMulticast_SetupMovementComp();
	
	
	// Setter

	UFUNCTION(Server, Reliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_SetIsShouldRun(const bool NewShouldRun);
	
	// Getter
	
	// Get is should run now
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="MovementInfo")
		bool GetIsShouldRun() const;
	
private:

	// Functions

	// Setup Movement Comp
	void SetupMovementComp_Internal();
	
	void CalculateMovementInfo();

	float GetDirection() const;
	
	// Calculate Max Speed by directions
	float GetMaxSpeedByDirection() const;
	void CalculateMaxSpeedByDirection();

	// Shake Functions

	// Start camera shake by speed
	void CalculateCameraShakeBySpeed();
	// Get camera shake by speed
	TSubclassOf<UCameraShakeBase> GetRecursiveCameraShakeBaseIndex(TArray<float> CameraShakeSpeedArray);
	// Stop current camera shake
	void StopCurrentCameraShake(APlayerCameraManager* PlayerCameraManager);
	// Start camera shake
	UCameraShakeBase* StartCameraShake(APlayerCameraManager* PlayerCameraManager, TSubclassOf<UCameraShakeBase> ShakeClass,
		 float Scale=1.f, ECameraShakePlaySpace PlaySpace = ECameraShakePlaySpace::CameraLocal,
		 FRotator UserPlaySpaceRot = FRotator::ZeroRotator);
	// Sort camera shake by speed
	void SortCameraShakeBySpeed();

	// Debug

	#if !UE_BUILD_SHIPPING
		void ShowDebug();
	#endif
	
	// Variables
	
	// Movement Info

	UPROPERTY(EditAnywhere, Category="MovementInfo", meta=(AllowPrivateAccess))
		TMap<float, TSubclassOf<UCameraShakeBase>> CameraShakeBySpeed;
	UPROPERTY(EditAnywhere, Category="MovementInfo", meta=(ClampMin="0.01", ClampMax="1", Units="s", AllowPrivateAccess))
		float CheckInfoFreq = 0.1f;
	UPROPERTY(EditAnywhere, Category="MovementInfo|Run", meta=(AllowPrivateAccess))
		float RunSpeed = 500.f;
	UPROPERTY(EditAnywhere, Category="MovementInfo|Run", meta=(AllowPrivateAccess))
		float RunSpeedCrouched = 250.f;

	// Service

	UPROPERTY(Replicated)
		float CurrentMaxSpeed = MaxWalkSpeed; // Current max speed
	UPROPERTY(Replicated)
		bool bShouldRun = false; // Is should run now
	
	FTimerHandle CalculateMovementInfoTimerHandle;
	
	// Camera Shake Info

	TObjectPtr<UCameraShakeBase> CurrentCameraShake = nullptr;

	// Show Debug

	UPROPERTY(EditAnywhere, Category="MovementInfo|Debug", meta=(AllowPrivateAccess))
		bool bShowDebug = false;
	
};
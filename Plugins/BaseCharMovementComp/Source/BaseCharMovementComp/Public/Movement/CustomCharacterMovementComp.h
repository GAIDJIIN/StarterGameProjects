// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "MovementComponentInteractInterface.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "CustomCharacterMovementComp.generated.h"

struct FActorComponentTickFunction;

USTRUCT(BlueprintType)
struct FMovementInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TEnumAsByte<EMovementMode> MovementModeMaxSpeed = EMovementMode::MOVE_Walking;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxSpeed = 500.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		float MaxCrouchingSpeed = 250.f;

	FMovementInfo() {}
	
};

USTRUCT(BlueprintType)
struct FMovementInfo_Array
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
		TArray<FMovementInfo> MovementInfo = {};
	
	// Get max movement speed by movement mode
	FORCEINLINE float GetMaxMovementSpeed(EMovementMode MovementMode, bool bIsCrouching, bool& bIsSuccess) const
	{
		bIsSuccess = false;
		for (const auto& Element : MovementInfo)
		{
			if (Element.MovementModeMaxSpeed == MovementMode)
			{
				bIsSuccess = true;
				return bIsCrouching ? Element.MaxCrouchingSpeed : Element.MaxSpeed;
			}
		}

		return 0.f;
	}

	FMovementInfo_Array() {}
	
};

UCLASS()
class BASECHARMOVEMENTCOMP_API UCustomCharacterMovementComp : public UCharacterMovementComponent, public IMovementComponentInteractInterface
{
	GENERATED_BODY()

public:

	// Override parents functions
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void Deactivate() override;
	virtual void BeginPlay() override;
	virtual float GetMaxSpeed() const override;

	// Interface Methods

	// Set current movement tag
	void ToggleCurrentMovementTag_Implementation(FGameplayTag NewMovementTag, bool bIsAdd = true);
	
	// Replicate properties
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	#if WITH_EDITOR
		virtual auto PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) -> void override;
	#endif
	
	// Reset Movement Comp
	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_ResetMovementComp();
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category="MovementInfo")
		void NetMulticast_ResetMovementComp();

	// Setup Movement Comp
	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_SetupMovementComp();
	UFUNCTION(NetMulticast, Unreliable, BlueprintCallable, Category="MovementInfo")
		void NetMulticast_SetupMovementComp();
	
private:

	// Functions

	// Setup Movement Comp
	void SetupMovementComp_Internal();
	
	void CalculateMovementInfo();

	float GetMaxSpeedByMovementMode() const;
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

	// RPC

	UFUNCTION(Server, Unreliable, WithValidation, BlueprintCallable, Category="MovementInfo")
		void Server_ToggleCurrentMovementTag(FGameplayTag NewMovementTag, bool bIsAdd = true);
	
	// Debug

	#if !UE_BUILD_SHIPPING
		void ShowDebug();
	#endif
	
	// Variables
	
	// Movement Info

	UPROPERTY(EditAnywhere, Category="MovementInfo", meta=(AllowPrivateAccess))
		TMap<float, TSubclassOf<UCameraShakeBase>> CameraShakeBySpeed;
	UPROPERTY(EditAnywhere, Category="MovementInfo", meta=(AllowPrivateAccess))
		TMap<FGameplayTag, FMovementInfo_Array> MaxWalkSpeedByTag;
	UPROPERTY(EditAnywhere, Category="MovementInfo", meta=(ClampMin="0.01", ClampMax="1", Units="s", AllowPrivateAccess))
		float CheckInfoFreq = 0.1f;

	// Service

	UPROPERTY(Replicated)
		FMovementInfo_Array CurrentMaxSpeedByMovementTag = FMovementInfo_Array();
	UPROPERTY(Replicated)
		float CurrentMaxSpeed = Super::GetMaxSpeed(); // Current max speed
	UPROPERTY(Replicated)
		FGameplayTagContainer CurrentMovementTags = FGameplayTagContainer();
	
	FTimerHandle CalculateMovementInfoTimerHandle;
	
	// Camera Shake Info

	TObjectPtr<UCameraShakeBase> CurrentCameraShake = nullptr;

	// Show Debug

	UPROPERTY(EditAnywhere, Category="MovementInfo|Debug", meta=(AllowPrivateAccess))
		bool bShowDebug = false;
	
};
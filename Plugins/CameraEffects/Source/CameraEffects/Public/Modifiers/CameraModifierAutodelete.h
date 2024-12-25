// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraModifier.h"
#include "CameraModifierAutodelete.generated.h"


UCLASS()
class CAMERAEFFECTS_API UCameraModifierAutodelete : public UCameraModifier
{
	GENERATED_BODY()

public:
	// Functions
	// Custom
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Auto Delete Camera Modifier")
		bool GetIsNeedDelete(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const;
	UFUNCTION(BlueprintCallable, Category = "Auto Delete Camera Modifier")
		void SetIsActive(bool bNewIsActive) { bIsActive = bNewIsActive; }
	UFUNCTION(BlueprintCallable, Category = "Auto Delete Camera Modifier")
		const bool GetIsActive() const { return bIsActive; }

	virtual bool GetIsNeedDelete_CPP(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const;

	// Override
	
	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;

private:
	bool bIsActive = true;
};

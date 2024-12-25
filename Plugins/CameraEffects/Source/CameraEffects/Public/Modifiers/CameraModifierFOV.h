// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "CameraModifierAutodelete.h"
#include "CameraModifierFOV.generated.h"


UCLASS()
class CAMERAEFFECTS_API UCameraModifierFOV : public UCameraModifierAutodelete
{
	GENERATED_BODY()
public:
	// Functions
	// Override
	
	virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV) override;
	virtual bool GetIsNeedDelete_CPP(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const override;
	
private:
	float CurrentFOV = 0.f;
	bool bIsWasFirstSet = false;

	// Blueprint
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier FOV", meta=(AllowPrivateAccess, ClampMin="4"))
		float TargetFOV = 60.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera Modifier FOV", meta=(AllowPrivateAccess, ClampMin="0"))
		float ExponentChangeFOV = 1.5f;
};

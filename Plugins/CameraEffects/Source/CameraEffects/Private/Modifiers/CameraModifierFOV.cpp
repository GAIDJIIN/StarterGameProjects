// Florist Game. All rights reserved.


#include "Modifiers/CameraModifierFOV.h"

#include "Kismet/KismetMathLibrary.h"

void UCameraModifierFOV::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
                                      FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	CurrentFOV = bIsWasFirstSet ? CurrentFOV : FOV;
	bIsWasFirstSet = true;
	CurrentFOV = UKismetMathLibrary::FInterpEaseInOut(CurrentFOV, GetIsActive() ? TargetFOV : FOV, 0.1f, ExponentChangeFOV);

	// Set new camera POV
	NewViewLocation = ViewLocation;
	NewViewRotation = ViewRotation;
	NewFOV = !UKismetMathLibrary::NearlyEqual_FloatFloat(CurrentFOV, FOV, 0.1f) || GetIsActive() ? CurrentFOV : FOV;

	Super::ModifyCamera(DeltaTime, ViewLocation, ViewRotation, FOV, NewViewLocation, NewViewRotation, NewFOV);
}

bool UCameraModifierFOV::GetIsNeedDelete_CPP(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const
{
	return UKismetMathLibrary::NearlyEqual_FloatFloat(CurrentFOV, FOV, 0.1f) && !GetIsActive();
}

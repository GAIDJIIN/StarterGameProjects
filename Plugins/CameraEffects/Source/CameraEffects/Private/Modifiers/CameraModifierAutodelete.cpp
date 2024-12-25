// Florist Game. All rights reserved.


#include "Modifiers/CameraModifierAutodelete.h"

bool UCameraModifierAutodelete::GetIsNeedDelete_Implementation(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const
{
	return GetIsNeedDelete_CPP(DeltaTime, ViewLocation, ViewRotation, FOV);
}

bool UCameraModifierAutodelete::GetIsNeedDelete_CPP(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV) const
{
	return false;
}

void UCameraModifierAutodelete::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
                                             FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV)
{
	Super::ModifyCamera(DeltaTime, ViewLocation, ViewRotation, FOV, NewViewLocation, NewViewRotation, NewFOV);
	if(GetIsNeedDelete(DeltaTime, ViewLocation, ViewRotation, FOV))
	{
		if(!CameraOwner) return;
		CameraOwner->RemoveCameraModifier(this);
	}
}

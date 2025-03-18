// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Interface.h"
#include "MovementComponentInteractInterface.generated.h"

UINTERFACE()
class UMovementComponentInteractInterface : public UInterface
{
	GENERATED_BODY()
};

class BASECHARMOVEMENTCOMP_API IMovementComponentInteractInterface
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="MovementInfoInterface")
		void ToggleCurrentMovementTag(FGameplayTag NewMovementTag, bool bIsAdd = true);
};

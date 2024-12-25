// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "ContainerForManagersBlueprintLibrary.generated.h"

class UContainerForManagersComp;

UCLASS()
class CONTAINERFORMANAGERS_API UContainerForManagersBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Get container for manager from game mode
	UFUNCTION(BlueprintCallable, Category="ContainerForManager")
		static UContainerForManagersComp* GetContainerForManagersFromGM(AGameModeBase* GameModeBase);

	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Class"), Category="ContainerForManager")
		static AActor* GetManagerFromGM(AGameModeBase* GameModeBase, TSubclassOf<AActor> Class);
};

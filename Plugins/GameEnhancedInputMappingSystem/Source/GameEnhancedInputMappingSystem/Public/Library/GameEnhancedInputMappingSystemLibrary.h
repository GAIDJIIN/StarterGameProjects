// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameEnhancedMappingInfo.h"
#include "GameEnhancedInputMappingSystemLibrary.generated.h"

/*
 * Game Enhanced Input Mapping System Library
 * All Methods for work with GameEnhancedInputMappingSystem
 */

class UGameEnhancedInputComponent;

UCLASS()
class GAMEENHANCEDINPUTMAPPINGSYSTEM_API UGameEnhancedInputMappingSystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:

	// Get Current Player Input Mode
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GameEnhancedInputMappingSystemLibrary|Getter")
		static const EPlayerInputMode GetCurrentPlayerInputMode(const APlayerController* PlayerController);
	// Get Game Enhanced Input Component from Player Controller
	UFUNCTION(BlueprintCallable, BlueprintPure, Category="GameEnhancedInputMappingSystemLibrary|Getter")
		static UPARAM(meta=(DisplayName="GameEnhancedInputComponent")) UGameEnhancedInputComponent* GetGameEnhancedInputComponent(const APlayerController* PlayerController);

};
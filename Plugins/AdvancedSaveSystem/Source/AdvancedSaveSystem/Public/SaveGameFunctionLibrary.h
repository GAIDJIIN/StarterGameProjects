// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SaveGameFunctionLibrary.generated.h"

UCLASS()
class ADVANCEDSAVESYSTEM_API USaveGameFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category="SaveGameFunctionLibrary")
		static bool WasObjectLoaded(UObject* Object);
};
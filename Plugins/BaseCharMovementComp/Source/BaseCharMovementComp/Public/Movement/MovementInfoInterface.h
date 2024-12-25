// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "MovementInfoInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UMovementInfoInterface : public UInterface
{
	GENERATED_BODY()
};


class BASECHARMOVEMENTCOMP_API IMovementInfoInterface
{
	GENERATED_BODY()

public:

	// Getter
	
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="MovementInfoInterface")
		bool GetIsShouldRun();
	
};

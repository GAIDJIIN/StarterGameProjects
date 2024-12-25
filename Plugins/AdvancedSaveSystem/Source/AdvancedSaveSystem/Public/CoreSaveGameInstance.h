// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "SaveSystem/CoreGameInstance.h"
#include "CoreSaveGameInstance.generated.h"

/*
 Core Save System Game Instance -
 use as parent for any child core game instance, if you need to use Advanced Save System
 */
UCLASS()
class ADVANCEDSAVESYSTEM_API UCoreSaveGameInstance : public UCoreGameInstance
{
	GENERATED_BODY()
};

// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GameEnhancedMappingInfo.h"
#include "DA_MainMappingContexts.generated.h"

/*
 * Contains main mapping contexts
 */

UCLASS()
class GAMEENHANCEDINPUTMAPPINGSYSTEM_API UDA_MainMappingContexts : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const TArray<FMappingContextInfo>& GetMainMappingContexts() const { return MainMappingContexts; }
	
private:
	UPROPERTY(EditAnywhere, Category="MappingContextInfo", meta=(AllowPrivateAccess))
		TArray<FMappingContextInfo> MainMappingContexts = {};
	
};

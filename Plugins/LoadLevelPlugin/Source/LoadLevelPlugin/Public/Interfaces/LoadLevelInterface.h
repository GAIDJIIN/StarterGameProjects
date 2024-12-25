// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "LoadLevelComponent.h"
#include "Engine/LevelStreaming.h"
#include "LoadLevelInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class ULoadLevelInterface : public UInterface
{
	GENERATED_BODY()
};

class ILoadLevelInterface
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Setter") void SetLastLevelLoadComponent(ULoadLevelComponent* LastLoadLevelComponent);
	virtual void SetLastLevelLoadComponent_Implementation(ULoadLevelComponent* LastLoadLevelComponent) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Getter") bool IsLevelLoaded(const FString& LevelTag);
	virtual bool IsLevelLoaded_Implementation(const FString& LevelTag) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Getter") bool IsLevelPendingLoad(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator);
	virtual bool IsLevelPendingLoad_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Getter") bool IsInstigatorHasPending(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator);
	virtual bool IsInstigatorHasPending_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Getter") ULoadLevelComponent* GetLastLevelLoadComponent() const;
	virtual ULoadLevelComponent* GetLastLevelLoadComponent_Implementation() const = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Getter") ULevelStreaming* GetLoadedLevelRef(const FString& LevelTag);
	virtual ULevelStreaming* GetLoadedLevelRef_Implementation(const FString& LevelTag) = 0;


	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void ToggleLoadLevel(const FString& LevelTag, bool bIsLoad, ULevelStreaming* LevelStreaming);
	virtual void ToggleLoadLevel_Implementation(const FString& LevelTag, bool bIsLoad, ULevelStreaming* LevelStreaming) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void ToggleLoadInstigator(const FString& LevelAssetName, bool bIsLoadLevel, UObject* LoadInstigator);
	virtual void ToggleLoadInstigator_Implementation(const FString& LevelAssetName, bool bIsLoadLevel, UObject* LoadInstigator) = 0;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable) void StartLoadingLevels();
	virtual void StartLoadingLevels_Implementation() = 0;
};

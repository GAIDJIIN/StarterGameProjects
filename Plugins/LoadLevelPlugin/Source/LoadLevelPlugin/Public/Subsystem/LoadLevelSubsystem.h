// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "LoadLevelInterface.h"
#include "LoadLevelInfo.h"
#include "LoadLevelSubsystem.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LoadLevelSystem, Log, All);

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelLoadSignature, const FString&, LevelAssetName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelsLoadingSignature);

UCLASS()
class LOADLEVELPLUGIN_API ULoadLevelSubsystem : public UWorldSubsystem, public ILoadLevelInterface
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintAssignable) FOnLevelLoadSignature OnLevelLoad;
	UPROPERTY(BlueprintAssignable) FOnLevelLoadSignature OnLevelUnload;
	UPROPERTY(BlueprintAssignable) FOnLevelsLoadingSignature OnStartLevelsLoading;

	void TickTimer();
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	
protected:
	ULoadLevelComponent* LastLoadedComponent;
	TMap<FString, FLoadedLevelStatus> AlreadyLoadedLevels;

	virtual void SetLastLevelLoadComponent_Implementation(ULoadLevelComponent* LastLoadLevelComponent) override { LastLoadedComponent = LastLoadLevelComponent; }
	virtual ULoadLevelComponent* GetLastLevelLoadComponent_Implementation() const override { return LastLoadedComponent; }

	virtual bool IsLevelLoaded_Implementation(const FString& LevelTag) override;
	virtual bool IsLevelPendingLoad_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator) override;
	virtual bool IsInstigatorHasPending_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator) override;
	
	FORCEINLINE virtual ULevelStreaming* GetLoadedLevelRef_Implementation(const FString& LevelTag) override 
	{ return AlreadyLoadedLevels.Contains(LevelTag) ? AlreadyLoadedLevels[LevelTag].LevelReference : nullptr; }

	virtual void ToggleLoadLevel_Implementation(const FString& LevelTag, bool bIsLoad, ULevelStreaming* LevelStreaming) override;
	virtual void ToggleLoadInstigator_Implementation(const FString& LevelAssetName, bool bIsLoadLevel, UObject* LoadInstigator) override;
	virtual void StartLoadingLevels_Implementation() override { OnStartLevelsLoading.Broadcast(); }
	
};

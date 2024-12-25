// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/LoadLevelSubsystem.h"

DEFINE_LOG_CATEGORY(LoadLevelSystem);

void ULoadLevelSubsystem::ToggleLoadInstigator_Implementation(const FString& LevelAssetName, bool bIsLoadLevel, UObject* LoadInstigator)
{
	FLoadedLevelStatus LoadedLevelInfoPtr;
	if (AlreadyLoadedLevels.Contains(LevelAssetName))
		LoadedLevelInfoPtr = AlreadyLoadedLevels[LevelAssetName];

	TArray<UObject*> LoadInstigators = LoadedLevelInfoPtr.PendingToLoadInstigators;
	TArray<UObject*> UnloadInstigators = LoadedLevelInfoPtr.PendingToUnLoadInstigators;

	if (bIsLoadLevel)
	{
		if (UnloadInstigators.Contains(LoadInstigator)) UnloadInstigators.Remove(LoadInstigator);
		else if (UnloadInstigators.IsEmpty()) LoadInstigators.AddUnique(LoadInstigator);
		else if (!UnloadInstigators.IsEmpty()) UnloadInstigators.RemoveAt(0);
	}
	else
	{
		if (LoadInstigators.Contains(LoadInstigator)) LoadInstigators.Remove(LoadInstigator);
		else if (LoadInstigators.IsEmpty()) UnloadInstigators.AddUnique(LoadInstigator);
		else if (!LoadInstigators.IsEmpty()) LoadInstigators.RemoveAt(0);
	}

	LoadedLevelInfoPtr.PendingToLoadInstigators = LoadInstigators;
	LoadedLevelInfoPtr.PendingToUnLoadInstigators = UnloadInstigators;

	AlreadyLoadedLevels.Add(LevelAssetName, LoadedLevelInfoPtr);
}

void ULoadLevelSubsystem::TickTimer()
{
	if(AlreadyLoadedLevels.IsEmpty()) return;
	for(auto pair : AlreadyLoadedLevels)
	{
		TArray<UObject*> LoadInstigators = pair.Value.PendingToLoadInstigators;
		TArray<UObject*> UnloadInstigators = pair.Value.PendingToUnLoadInstigators;
		for(auto LocalLoadInstigator : LoadInstigators);
		for(auto LocalUnLoadInstigator : UnloadInstigators);
	}
}

void ULoadLevelSubsystem::OnWorldBeginPlay(UWorld& InWorld)
{
	FTimerHandle TimerHandle;
	InWorld.GetTimerManager().SetTimer(TimerHandle,this, &ULoadLevelSubsystem::TickTimer,0.01f,true);
}

bool ULoadLevelSubsystem::IsLevelLoaded_Implementation(const FString& LevelTag)
{
	if (!AlreadyLoadedLevels.Contains(LevelTag)) return false;
	return AlreadyLoadedLevels.FindRef(LevelTag).bIsLoaded;
}

bool ULoadLevelSubsystem::IsLevelPendingLoad_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator)
{
	FLoadedLevelStatus LoadedLevelInfoPtr;
	if (AlreadyLoadedLevels.Contains(LevelTag))
		LoadedLevelInfoPtr = AlreadyLoadedLevels[LevelTag];
	
	TArray<UObject*> LoadInstigators = LoadedLevelInfoPtr.PendingToLoadInstigators;
	TArray<UObject*> UnloadInstigators = LoadedLevelInfoPtr.PendingToUnLoadInstigators;
	
	if (bIsPendingLoad) UnloadInstigators.Remove(LoadInstigator);
	else LoadInstigators.Remove(LoadInstigator);

	for(auto LocalLoadInstigator : LoadInstigators);
	for(auto LocalUnLoadInstigator : UnloadInstigators);
	UE_LOG(LoadLevelSystem, Log, TEXT("\n\nUnLoad Num - %i"), UnloadInstigators.Num());
	UE_LOG(LoadLevelSystem, Log, TEXT("\nLoad Num - %i"), LoadInstigators.Num());
	
	return LoadInstigators.IsEmpty() && UnloadInstigators.IsEmpty();
}

bool ULoadLevelSubsystem::IsInstigatorHasPending_Implementation(const FString& LevelTag, bool bIsPendingLoad, UObject* LoadInstigator)
{
	FLoadedLevelStatus LoadedLevelInfoPtr;
	if (AlreadyLoadedLevels.Contains(LevelTag))
		LoadedLevelInfoPtr = AlreadyLoadedLevels[LevelTag];

	TArray<UObject*> LoadInstigators = LoadedLevelInfoPtr.PendingToLoadInstigators;
	TArray<UObject*> UnloadInstigators = LoadedLevelInfoPtr.PendingToUnLoadInstigators;

	return bIsPendingLoad ? LoadInstigators.Contains(LoadInstigator) : UnloadInstigators.Contains(LoadInstigator);
}

void ULoadLevelSubsystem::ToggleLoadLevel_Implementation(const FString& LevelTag, bool bIsLoad, ULevelStreaming* LevelStreaming)
{
	FLoadedLevelStatus LoadedLevelInfoPtr;
	if (AlreadyLoadedLevels.Contains(LevelTag))
		LoadedLevelInfoPtr = AlreadyLoadedLevels[LevelTag];

	LoadedLevelInfoPtr.bIsLoaded = bIsLoad;
	LoadedLevelInfoPtr.LevelReference = LevelStreaming;

	AlreadyLoadedLevels.Add(LevelTag, LoadedLevelInfoPtr);

	if (bIsLoad) OnLevelLoad.Broadcast(LevelTag);
	else OnLevelUnload.Broadcast(LevelTag);
}
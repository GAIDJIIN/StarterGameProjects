// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "LoadLevelInfo.h"
#include "LoadLevelComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LoadLevelComponent, Log, All);

class ULoadLevelSubsystem;
class ALevelInstance;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadLevelsSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnUnloadLevelsSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadAlreadyLoadedLevelsSignature);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLevelLoadedSignature);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class LOADLEVELPLUGIN_API ULoadLevelComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable) FOnLoadLevelsSignature OnLoadLevels;
	UPROPERTY(BlueprintAssignable) FOnUnloadLevelsSignature OnUnloadLevels;
	UPROPERTY(BlueprintAssignable) FOnLoadAlreadyLoadedLevelsSignature OnLoadAlreadyLoadedLevels;
	UPROPERTY(BlueprintAssignable) FOnLevelLoadedSignature OnLevelLoaded;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bIsLoadLevelNow = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bLoadOneLevel = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Info|Execute Commands") TArray<FString> OnLoadLevelCommands;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Level Info|Execute Commands") TArray<FString> OnUnLoadLevelCommands;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Info") float TimeToHideLVL = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Info") TArray<FLoadLevelInfo> NeedLoadLevels;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Info") TArray<FForToggleLevelInfo> AlreadyLoadedLevels;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Level Info") float TimeLimitCheckAlreadyLoadedLevels = 10.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") FTimerHandle UnLoadLevelTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") TMap<FString, ULevelStreaming*> LoadedLevels;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") TMap<FString, ULevelStreaming*> QueueNeedLoadLevels;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") TMap<FString, ALevelInstance*> QueueAlreadyLoadedLevels;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") FTimerHandle QueueLoadedTimerHandle;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") FTimerHandle QueueLoadedLimitTimerHandle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bIsLoadedStateNow = false;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bIsAlreadyCallLoadLevelNow = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") FTimerHandle ExecuteCommandTimerHandle;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Service") float TimeToExecuteCommand = 0.5f;
	

	UObject* LoadLevelSubsystemRef = nullptr;

//---------------------------------- METHODS ------------------------------------//
public:
	ULoadLevelComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Load Level") ULevelStreaming* GetLevelStreamingFromInstance(ALevelInstance* LevelInstance) const;

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter") FString ConvertLevelTag(FLoadLevelInfo LoadLevelInfo);
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter") FORCEINLINE FString ConvertLevelTagByRef(ALevelInstance* LevelInstance);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Getter") FORCEINLINE bool GetIsLoadedStateNow() { return bIsLoadedStateNow; }

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void LoadLevels();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void UnloadLevels();

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Execute Commands") void ExecuteCommandsDeffered(float Time, bool bIsLoad);

	UFUNCTION(BlueprintCallable) void OnLoadAlreadyLoadedLevels_Event();
	UFUNCTION(BlueprintCallable) void StopLoadAlreadyLoadedLevels();

	UFUNCTION(BlueprintCallable) void CheckIsLevelLoaded(ALevelInstance* LevelInstance);

	UFUNCTION(BlueprintCallable, BlueprintPure) bool IsLevelsValid();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void UnloadLevelsMainLogic();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void OnLoadLevel();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void InitializeAlreadyLoadLevels();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic") void LoadLevelsMainLogic();

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Execute Commands") void ExecuteCommands(TArray<FString> CommandsToExecute);
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Execute Commands") FORCEINLINE void ExecuteCommandsOnLoad() { ExecuteCommands(OnLoadLevelCommands); }
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Execute Commands") FORCEINLINE void ExecuteCommandsOnUnload() { ExecuteCommands(OnUnLoadLevelCommands); }

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Main Levels") void UnloadLevelsLogic();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Main Levels") void LoadLevelsLogic();

	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Already") void UnloadAlreadyLoadedLevels();
	UFUNCTION(BlueprintCallable, Category = "Load Level Logic|Already") void LoadAlreadyLoadedLevels();

	void LoadLevelInstanceByObjRef(FLoadLevelInfo LoadLevelInfo, const FString& LevelTag);
};

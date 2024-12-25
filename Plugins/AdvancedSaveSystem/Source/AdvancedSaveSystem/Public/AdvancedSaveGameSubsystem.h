// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "AdvancedSaveGameSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLoadCompleted);

/*
 *System for save/load objects in world
*/

UCLASS()
class ADVANCEDSAVESYSTEM_API UAdvancedSaveGameSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// Override
	
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// Variables
	
	UPROPERTY(BlueprintAssignable, Category="AdvancedSaveSystem")
		FOnLoadCompleted OnLoadCompleted;

	// Functions
	
	UFUNCTION(BlueprintCallable, Category="AdvancedSaveSystem")
		bool Save();
	UFUNCTION(BlueprintCallable, Category="AdvancedSaveSystem")
		bool Load();
	UFUNCTION(BlueprintCallable, Category="AdvancedSaveSystem")
		bool GetIsLoadingSaveGame();
	
protected:
	void OnWorldInitializationEvent(UWorld* World, const UWorld::InitializationValues IVS);
	void OnWorldInitializedActorsEvent(const FActorsInitializedParams& Params);
	void OnWorldCleanupEvent(UWorld* World, bool bSessionEnded, bool bCleanupResources);

	void OnActorPreSpawn(AActor* Actor);
	void OnActorDestroy(AActor* Actor);

	UFUNCTION(BlueprintNativeEvent, Category="AdvancedSaveSystem")
		void OnLoadCompletedLogic();
	
private:

	// Variables
	
	TSet<TWeakObjectPtr<AActor>> SavedActors;
	TSet<FSoftObjectPath> DestroyedActors;

	// Functions

	void CallOnLoadCompleted();
};
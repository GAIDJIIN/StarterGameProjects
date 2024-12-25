// Florist Game. All rights reserved.


#include "AdvancedSaveGameSubsystem.h"
#include "SaveGameFunctionLibrary.h"
#include "EngineUtils.h"
#include "SaveGameInterface.h"


void UAdvancedSaveGameSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &ThisClass::OnWorldInitializationEvent);
	FWorldDelegates::OnWorldInitializedActors.AddUObject(this, &ThisClass::OnWorldInitializedActorsEvent);
	FWorldDelegates::OnWorldCleanup.AddUObject(this, &ThisClass::OnWorldCleanupEvent);

	/*
	 * Level Streaming Delegates
	 * FWorldDelegates::LevelAddedToWorld and FWorldDelegates::PreLevelRemovedFromWorld
	 */

	// Load main current level
	OnWorldInitializationEvent(GetWorld(), UWorld::InitializationValues());
}

void UAdvancedSaveGameSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

//--------------------------------------------------DELEGATES---------------------------------------------------------//
void UAdvancedSaveGameSubsystem::OnWorldInitializationEvent(UWorld* World, const UWorld::InitializationValues IVS)
{
	if(!World || World != GetWorld()) return;

	// Bind On Actors Spawn/Destroy
	World->AddOnActorPreSpawnInitialization(FOnActorSpawned::FDelegate::CreateUObject(this, &ThisClass::OnActorPreSpawn));
	World->AddOnActorDestroyedHandler(FOnActorDestroyed::FDelegate::CreateUObject(this, &ThisClass::OnActorDestroy));
}

void UAdvancedSaveGameSubsystem::OnWorldInitializedActorsEvent(const FActorsInitializedParams& Params)
{
	if(!Params.World || GetWorld() != Params.World) return;

	for(TActorIterator<AActor> It(Params.World); It; ++It )
	{
		AActor* Actor = *It;
		OnActorPreSpawn(Actor);
	}
}

void UAdvancedSaveGameSubsystem::OnWorldCleanupEvent(UWorld* World, bool bSessionEnded, bool bCleanupResources)
{
	if(World || GetWorld() != World) return;

	SavedActors.Reset();
	DestroyedActors.Reset();
}

void UAdvancedSaveGameSubsystem::OnActorPreSpawn(AActor* Actor)
{
	if(Actor && Actor->Implements<USaveGameInterface>()) SavedActors.Add(Actor);
}

void UAdvancedSaveGameSubsystem::OnActorDestroy(AActor* Actor)
{
	SavedActors.Remove(Actor);
	if(USaveGameFunctionLibrary::WasObjectLoaded(Actor)) DestroyedActors.Add(Actor);
}

//---------------------------------------------------Save/Load--------------------------------------------------------//
bool UAdvancedSaveGameSubsystem::Save()
{
	return false;
}

bool UAdvancedSaveGameSubsystem::Load()
{
	return false;
}

bool UAdvancedSaveGameSubsystem::GetIsLoadingSaveGame()
{
	return false;
}

void UAdvancedSaveGameSubsystem::CallOnLoadCompleted()
{
	OnLoadCompletedLogic();
	OnLoadCompleted.Broadcast();
}

void UAdvancedSaveGameSubsystem::OnLoadCompletedLogic_Implementation()
{
	
}

//--------------------------------------------------------------------------------------------------------------------//
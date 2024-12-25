// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadLevelComponent.h"
#include "Interfaces/LoadLevelInterface.h"
#include "Engine/LevelStreaming.h"
#include "LevelInstance/LevelInstanceLevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "GameFramework/GameStateBase.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

DEFINE_LOG_CATEGORY(LoadLevelComponent);

// Sets default values for this component's properties
ULoadLevelComponent::ULoadLevelComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	TimeToHideLVL = 2.f;
	TimeLimitCheckAlreadyLoadedLevels = 10.f;
	TimeToExecuteCommand = 0.5f;
	bIsLoadedStateNow = false;
	bIsAlreadyCallLoadLevelNow = false;
	bIsLoadLevelNow = false;
	bLoadOneLevel = false;

}


// Called when the game starts
void ULoadLevelComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!GetWorld()) return;
	LoadLevelSubsystemRef = Cast<UObject>(GetWorld()->GetSubsystem<ULoadLevelSubsystem>());
	checkf(LoadLevelSubsystemRef, TEXT("No Load Subsystem!!!!"))
}

void ULoadLevelComponent::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	ExecuteCommandsOnUnload();
}


// Called every frame
void ULoadLevelComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsLoadedStateNow) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Purple, *FString::Printf(TEXT("%s: %i"), *GetNameSafe(this), bIsLoadedStateNow));
}

ULevelStreaming* ULoadLevelComponent::GetLevelStreamingFromInstance(ALevelInstance* LevelInstance) const
{
	if (!LevelInstance) return nullptr;
	auto LocalLevelStreaming = Cast<ULevelStreaming>(LevelInstance->GetLevelStreaming());
	return LocalLevelStreaming;
}

FString ULoadLevelComponent::ConvertLevelTag(FLoadLevelInfo LoadLevelInfo)
{
	if (!LoadLevelInfo.LevelTag.IsEmpty()) return LoadLevelInfo.LevelTag;

	// TO DO: Async asset load
	FString str = LoadLevelInfo.Level.WorldAsset.GetAssetName() + " " + LoadLevelInfo.LevelLocation.ToString() + " " + LoadLevelInfo.LevelRotation.ToString();
	return FString::FromInt((int32)CityHash32(TCHAR_TO_ANSI(*str), str.Len()));
}

FString ULoadLevelComponent::ConvertLevelTagByRef(ALevelInstance* LevelInstance)
{
	return LevelInstance->GetName();
}

void ULoadLevelComponent::LoadLevels()
{
	if (!IsLevelsValid()) return;

	//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->StartLoadingLevels_Implementation();
	ILoadLevelInterface::Execute_StartLoadingLevels(LoadLevelSubsystemRef);

	bIsLoadedStateNow = true;
	bIsLoadLevelNow = true;

	GetWorld()->GetTimerManager().ClearTimer(UnLoadLevelTimerHandle);
	ULoadLevelComponent* LocalLastLoadLevelComp = ILoadLevelInterface::Execute_GetLastLevelLoadComponent(LoadLevelSubsystemRef); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->GetLastLevelLoadComponent_Implementation();
	if (IsValid(LocalLastLoadLevelComp) && LocalLastLoadLevelComp->GetIsLoadedStateNow() && !NeedLoadLevels.IsEmpty())
		OnLoadAlreadyLoadedLevels.AddDynamic(this, &ThisClass::LoadLevelsMainLogic);
	else LoadLevelsMainLogic();
}

void ULoadLevelComponent::UnloadLevels()
{
	bIsLoadLevelNow = false;
	bool bIsHaveLoadLevel = false;
	ExecuteCommandsDeffered(TimeToExecuteCommand, bIsLoadLevelNow);
	
	for (auto pair : LoadedLevels)
	{
		if (!IsValid(pair.Value)) continue;
		if (!pair.Value->IsLevelLoaded()) continue;

		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(pair.Key, false, this);
		ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, pair.Key, false, this);

		if (!bIsHaveLoadLevel) bIsHaveLoadLevel = true;
	}

	if (!NeedLoadLevels.IsEmpty())
		GetWorld()->GetTimerManager().SetTimer(UnLoadLevelTimerHandle, this, &ThisClass::UnloadLevelsMainLogic, TimeToHideLVL, false);
	else UnloadLevelsMainLogic();

	InitializeAlreadyLoadLevels();
	LoadAlreadyLoadedLevels();
}

void ULoadLevelComponent::UnloadLevelsMainLogic()
{
	UnloadLevelsLogic();
	OnUnloadLevels.Broadcast();
}

void ULoadLevelComponent::OnLoadLevel()
{
	auto LevelTagsToRemove = QueueNeedLoadLevels;
	
	for (auto pair : QueueNeedLoadLevels)
	{
		if (!pair.Value->IsLevelLoaded()) continue;
		
		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(pair.Key, true, this);
		LevelTagsToRemove.Remove(pair.Key);
		ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, pair.Key, true, this);
		pair.Value->OnLevelShown.RemoveDynamic(this, &ThisClass::OnLoadLevel);

		//LevelTagsToRemove.AddUnique(pair.Key);
		//QueueNeedLoadLevels.Remove(pair.Key);
	}

	QueueNeedLoadLevels = LevelTagsToRemove;
	LevelTagsToRemove.Empty();
	
	if (QueueNeedLoadLevels.IsEmpty() && !bIsAlreadyCallLoadLevelNow)
	{
		bLoadOneLevel = false;
		OnLoadLevels.Broadcast();
		bIsAlreadyCallLoadLevelNow = true;
	}
}

void ULoadLevelComponent::InitializeAlreadyLoadLevels()
{
	for (auto item : AlreadyLoadedLevels)
	{
		if(!item.LevelRef) continue;
		FString LocalLevelTag = ConvertLevelTagByRef(item.LevelRef.Get());
		if (IsValid(item.LevelRef.Get()))
		{
			bool LocalLevelLoaded = item.LevelRef.Get()->IsLoaded();
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, LocalLevelLoaded, LocalLevelLoaded ? GetLevelStreamingFromInstance(item.LevelRef) : nullptr);
			ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, LocalLevelLoaded, LocalLevelLoaded ? GetLevelStreamingFromInstance(item.LevelRef.Get()) : nullptr);
		}
		else ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, false, nullptr); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, false, nullptr); 
	}
}

void ULoadLevelComponent::LoadLevelsMainLogic()
{
	OnLoadAlreadyLoadedLevels.RemoveDynamic(this, &ThisClass::LoadLevelsMainLogic);

	if (NeedLoadLevels.IsEmpty() && AlreadyLoadedLevels.IsEmpty()) return;

	bIsAlreadyCallLoadLevelNow = false;
	LoadLevelsLogic();
	InitializeAlreadyLoadLevels();
	UnloadAlreadyLoadedLevels();
	if (!AlreadyLoadedLevels.IsEmpty()) ILoadLevelInterface::Execute_SetLastLevelLoadComponent(LoadLevelSubsystemRef, this); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->SetLastLevelLoadComponent_Implementation(this);
	else bIsLoadedStateNow = false;
	ExecuteCommandsDeffered(TimeToExecuteCommand, bIsLoadLevelNow);
}

void ULoadLevelComponent::UnloadLevelsLogic()
{
	auto OutKeys = LoadedLevels;
	
	for (auto pair : LoadedLevels)
	{
		FString LocalLevelTag = pair.Key;
		bool bSuccess = ILoadLevelInterface::Execute_IsLevelPendingLoad(LoadLevelSubsystemRef, LocalLevelTag, false, this); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelPendingLoad_Implementation(LocalLevelTag, false, this);
		UE_LOG(LoadLevelComponent, Log, TEXT("Execute_IsLevelPendingLoadCheck"));
		if (!bSuccess) continue;
		UE_LOG(LoadLevelComponent, Log, TEXT("Execute_IsLevelPendingLoadTrueTrueTrue"));
		OutKeys.Remove(LocalLevelTag);
		if (IsValid(pair.Value))
		{
			pair.Value->SetShouldBeLoaded(false);
			UE_LOG(LoadLevelComponent, Log, TEXT("\nUnload Level : %s"), *pair.Value->GetName());
		}
		
		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, false, nullptr); 
		ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, false, pair.Value);
		GetWorld()->GetTimerManager().ClearTimer(UnLoadLevelTimerHandle);
	}

	LoadedLevels = OutKeys;
}

void ULoadLevelComponent::LoadLevelsLogic()
{
	if (!QueueNeedLoadLevels.IsEmpty()) return;

	for (auto item : NeedLoadLevels)
	{
		FString LocalLevelTag = ConvertLevelTag(item);
		bool LocalLevelLoaded = ILoadLevelInterface::Execute_IsLevelLoaded(LoadLevelSubsystemRef, LocalLevelTag); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelLoaded_Implementation(LocalLevelTag); 
		bool LocalLevelPendingLoaded = ILoadLevelInterface::Execute_IsLevelPendingLoad(LoadLevelSubsystemRef, LocalLevelTag, true, this); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelPendingLoad_Implementation(LocalLevelTag, true, this); 

		if (!LocalLevelLoaded && LocalLevelPendingLoaded)
		{
			if (LoadedLevels.Contains(LocalLevelTag) && IsValid(LoadedLevels[LocalLevelTag]))
			{
				ULevelStreaming* LocalLoadedLevel = LoadedLevels[LocalLevelTag];
				if (LocalLoadedLevel->IsLevelLoaded())
				{
					LocalLoadedLevel->SetShouldBeLoaded(true);
					LocalLoadedLevel->SetShouldBeVisible(true);
					UE_LOG(LoadLevelComponent, Log, TEXT("\nLoad Level : %s"), *LocalLoadedLevel->GetName());
					bLoadOneLevel = true;
					
					//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, true, LocalLoadedLevel);
					ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, true, LocalLoadedLevel);
				}
				else if (!item.Level.WorldAsset.IsNull()) LoadLevelInstanceByObjRef(item, LocalLevelTag);
			}
			else if (!item.Level.WorldAsset.IsNull()) LoadLevelInstanceByObjRef(item, LocalLevelTag);
		}
		else
		{
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(LocalLevelTag, true, this); 
			ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, LocalLevelTag, true, this);
			LoadedLevels.Add(LocalLevelTag, ILoadLevelInterface::Execute_GetLoadedLevelRef(LoadLevelSubsystemRef, LocalLevelTag)); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->GetLoadedLevelRef_Implementation(LocalLevelTag)
		}
	}

	for (auto pair : QueueNeedLoadLevels)
	{
		pair.Value->SetShouldBeLoaded(true);
		pair.Value->SetShouldBeVisible(true);
		UE_LOG(LoadLevelComponent, Log, TEXT("\nLoad Level : %s"), *pair.Value->GetName());
	}

	if (QueueNeedLoadLevels.IsEmpty() && !bIsAlreadyCallLoadLevelNow)
	{
		bLoadOneLevel = false;
		OnLoadLevels.Broadcast();
		bIsAlreadyCallLoadLevelNow = true;
	}
}

void ULoadLevelComponent::UnloadAlreadyLoadedLevels()
{
	for (auto item : AlreadyLoadedLevels)
	{
		if (!item.LevelRef) continue;
		FString LocalLevelTag = ConvertLevelTagByRef(item.LevelRef.Get());
		if (!ILoadLevelInterface::Execute_IsLevelPendingLoad(LoadLevelSubsystemRef, LocalLevelTag, false, this)) //(Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelPendingLoad_Implementation(LocalLevelTag, false, this))
		{
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(LocalLevelTag, false, this); 
			ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, LocalLevelTag, false, this);
			return;
		}
		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, false, nullptr); 
		ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, false, nullptr);
		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(LocalLevelTag, false, this);
		ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, LocalLevelTag, false, this);

		if (item.LevelRef.Get()) item.LevelRef.Get()->UnloadLevelInstance();
	}
}

void ULoadLevelComponent::LoadAlreadyLoadedLevels()
{
	for (auto item : AlreadyLoadedLevels)
	{
		if (!item.LevelRef) continue;
		FString LocalLevelTag = ConvertLevelTagByRef(item.LevelRef.Get());
		bool LocalLevelLoad = ILoadLevelInterface::Execute_IsLevelLoaded(LoadLevelSubsystemRef, LocalLevelTag); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelLoaded_Implementation(LocalLevelTag); //
		bool LocalLevelPendingLoad = ILoadLevelInterface::Execute_IsLevelPendingLoad(LoadLevelSubsystemRef, LocalLevelTag, true, this); //Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->IsLevelPendingLoad_Implementation(LocalLevelTag, true, this); //

		if (!LocalLevelLoad && LocalLevelPendingLoad)
		{
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LocalLevelTag, true, GetLevelStreamingFromInstance(item.LevelRef));
			ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LocalLevelTag, true, GetLevelStreamingFromInstance(item.LevelRef.Get()));
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadInstigator_Implementation(LocalLevelTag, true, this); 
			ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, LocalLevelTag, true, this);
			if (IsValid(item.LevelRef.Get()))
			{
				item.LevelRef.Get()->LoadLevelInstance();
				QueueAlreadyLoadedLevels.Add(LocalLevelTag, item.LevelRef.Get());
			}
		}
		else ILoadLevelInterface::Execute_ToggleLoadInstigator(LoadLevelSubsystemRef, LocalLevelTag, true, this);
	}


	// Check Is All Already Loaded Levels Load

	if (!QueueAlreadyLoadedLevels.IsEmpty())
	{
		FTimerManager& TimerManager = GetWorld()->GetTimerManager();
		TimerManager.SetTimer(QueueLoadedTimerHandle, this, &ThisClass::OnLoadAlreadyLoadedLevels_Event, 0.1f, true, 0.f);
		TimerManager.SetTimer(QueueLoadedLimitTimerHandle, this, &ThisClass::StopLoadAlreadyLoadedLevels, TimeLimitCheckAlreadyLoadedLevels, false, 0.f);
	}
	else StopLoadAlreadyLoadedLevels();
}

void ULoadLevelComponent::LoadLevelInstanceByObjRef(FLoadLevelInfo LoadLevelInfo, const FString& LevelTag)
{
	bool LocalSuccess = false;
	ULevelStreaming* LocalLoadedLevel = ULevelStreamingDynamic::LoadLevelInstanceBySoftObjectPtr(
		GetWorld(),
		LoadLevelInfo.Level.WorldAsset,
		LoadLevelInfo.LevelLocation,
		LoadLevelInfo.LevelRotation,
		LocalSuccess);

	LoadedLevels.Add(LevelTag, LocalLoadedLevel);

	if (IsValid(LocalLoadedLevel))
	{
		if (!LocalLoadedLevel->IsLevelLoaded())
		{
			QueueNeedLoadLevels.Add(LevelTag, LocalLoadedLevel);
			bLoadOneLevel = true;
			LocalLoadedLevel->OnLevelShown.AddDynamic(this, &ThisClass::OnLoadLevel);
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LevelTag, true, LocalLoadedLevel); 
			ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LevelTag, true, LocalLoadedLevel);
		}
		else
		{
			LocalLoadedLevel->SetShouldBeLoaded(true);
			LocalLoadedLevel->SetShouldBeVisible(true);
			UE_LOG(LoadLevelComponent, Log, TEXT("\nLoad Level : %s"), *LocalLoadedLevel->GetName());
			bLoadOneLevel = false;
			//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LevelTag, true, LocalLoadedLevel);
			ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LevelTag, true, LocalLoadedLevel);
		}
	}
	else
	{
		LocalLoadedLevel->SetShouldBeLoaded(true);
		LocalLoadedLevel->SetShouldBeVisible(true);
		UE_LOG(LoadLevelComponent, Log, TEXT("\nLoad Level : %s"), *LocalLoadedLevel->GetName());
		bLoadOneLevel = false;
		//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->ToggleLoadLevel_Implementation(LevelTag, true, LocalLoadedLevel);  
		ILoadLevelInterface::Execute_ToggleLoadLevel(LoadLevelSubsystemRef, LevelTag, true, LocalLoadedLevel);
	}
}

void ULoadLevelComponent::ExecuteCommands(TArray<FString> CommandsToExecute)
{
	for (auto command : CommandsToExecute)
	{
		if (!command.IsEmpty())
			UKismetSystemLibrary::ExecuteConsoleCommand(
				GetWorld(), 
				command,
				UGameplayStatics::GetPlayerController(GetWorld(), 0) );
	}
}

void ULoadLevelComponent::ExecuteCommandsDeffered(float Time, bool bIsLoad)
{
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(ExecuteCommandTimerHandle);
	TimerManager.SetTimer(
		ExecuteCommandTimerHandle,
		this,
		bIsLoad ? &ThisClass::ExecuteCommandsOnLoad : &ThisClass::ExecuteCommandsOnUnload,
		FMath::Max(Time, 0.00001f),
		false,
		0.f);
}

void ULoadLevelComponent::OnLoadAlreadyLoadedLevels_Event()
{
	if (QueueAlreadyLoadedLevels.IsEmpty()) { StopLoadAlreadyLoadedLevels(); return; }

	for (auto pair : QueueAlreadyLoadedLevels)
	{
		if (!IsValid(pair.Value)) return;
		if (!Cast<ILevelInstanceInterface>(pair.Value)->IsLoaded()) return;
		QueueAlreadyLoadedLevels.Remove(pair.Key);
		if (QueueAlreadyLoadedLevels.IsEmpty()) { StopLoadAlreadyLoadedLevels(); return; }
	}
}

void ULoadLevelComponent::StopLoadAlreadyLoadedLevels()
{
	bIsLoadedStateNow = false;
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(QueueLoadedTimerHandle);
	TimerManager.ClearTimer(QueueLoadedLimitTimerHandle);
	QueueAlreadyLoadedLevels.Empty();
	
	OnLoadAlreadyLoadedLevels.Broadcast();
	OnLoadAlreadyLoadedLevels.Clear();

	//Cast<ILoadLevelInterface>(LoadLevelSubsystemRef)->SetLastLevelLoadComponent_Implementation(this); 
	ILoadLevelInterface::Execute_SetLastLevelLoadComponent(Cast<UObject>(LoadLevelSubsystemRef), this);

	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 2.f, FColor::White, *FString::Printf(TEXT("%s"), *GetNameSafe(this)));
}

void ULoadLevelComponent::CheckIsLevelLoaded(ALevelInstance* LevelInstance)
{
	if (LevelInstance)
	{
		ALevelInstance* CurrentLevelInstance = LevelInstance;
		if (LevelInstance->IsLoaded()) OnLevelLoaded.Broadcast();
		else
		{
			FTimerHandle LocalTimerHandle;
			FTimerDelegate LocalTimerDelegate = FTimerDelegate::CreateUFunction(this, FName("CheckIsLevelLoaded"), CurrentLevelInstance);
			GetWorld()->GetTimerManager().SetTimer(LocalTimerHandle, LocalTimerDelegate, 0.2f, true);
		}
	}
}

bool ULoadLevelComponent::IsLevelsValid()
{
	for (auto item : NeedLoadLevels)
		if (!item.Level.WorldAsset.IsNull()) return true;
	for (auto alreadyloadedlevel : AlreadyLoadedLevels)
		if(alreadyloadedlevel.LevelRef.IsValid()) return true;
		
	return false;
}


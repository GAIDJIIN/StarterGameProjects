// Florist Game. All rights reserved.


#include "Component/GameEnhancedInputComponent.h"

#include "DA_MainMappingContexts.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameEnhancedMappingInfo.h"

UGameEnhancedInputComponent::UGameEnhancedInputComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UGameEnhancedInputComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                                FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ShowDebugInfo();
}

void UGameEnhancedInputComponent::BeginPlay()
{
	Super::BeginPlay();
	
	const auto LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0);
	if(LocalPlayerController && LocalPlayerController->GetLocalPlayer())
	{
		EnhancedInputLocalPlayerSubsystem = LocalPlayerController->GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();
	}
	checkf(EnhancedInputLocalPlayerSubsystem.Get(), TEXT("No find EnhancedInputLocalPlayerSubsystem"))

	if(!bInitializeMainMappingContextsOnBeginPlay) return;
	SetMainMappingContexts(CurrentMainMappingContextsDA,  FReplaceMainMappingContextsOptions(true, false, false));
}

void UGameEnhancedInputComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
	RemoveAllMappingContexts();
}

const bool UGameEnhancedInputComponent::SetMappingContext(const FMappingContextInfo& MappingContextInfo,
	const bool bIsAdd, const bool bIsCacheRemovedMapping, const FModifyContextOptions& Options)
{
	if(!GetCanWorkWithEnhancedInputSubsystem() || !MappingContextInfo.InputMappingContext) return false;
	
	// Find Mapping index already contains in AddedMappingContexts
	int32 LocalAddedMappingIndex = AddedMappingContexts.IndexOfByPredicate([&](const FMappingContextInfo& AddedMappingContext)
	{
		return AddedMappingContext.InputMappingContext == MappingContextInfo.InputMappingContext;
	});

	// Return false if on add already have mapping context or on remove no mapping context
	if(bIsAdd ? LocalAddedMappingIndex >= 0 : LocalAddedMappingIndex < 0) return false;
	
	// Add or Remove Mapping Context
	if(bIsAdd)
	{
		AddedMappingContexts.Add(MappingContextInfo);
		EnhancedInputLocalPlayerSubsystem->AddMappingContext(MappingContextInfo.InputMappingContext,MappingContextInfo.InputPriority,Options);
		CachedMappingContexts.Remove(MappingContextInfo);
		OnAddMappingContext.Broadcast(MappingContextInfo.InputMappingContext);
	}
	else
	{
		AddedMappingContexts.RemoveAt(LocalAddedMappingIndex);
		EnhancedInputLocalPlayerSubsystem->RemoveMappingContext(MappingContextInfo.InputMappingContext,Options);
		// Add in cache array removed mapping context for restore it later
		if(bIsCacheRemovedMapping) CachedMappingContexts.Add(MappingContextInfo);
		OnRemoveMappingContext.Broadcast(MappingContextInfo.InputMappingContext);
	}
	
	return true;
}

void UGameEnhancedInputComponent::RestoreCachedMappingContexts(const TArray<EMappingContextState>& StatesToRestore)
{
	if(CachedMappingContexts.IsEmpty()) return;
	TArray<FMappingContextInfo> LocalMappingContextsToRestore = CachedMappingContexts;
	if(!StatesToRestore.IsEmpty())
	{
		LocalMappingContextsToRestore = CachedMappingContexts.FilterByPredicate([&](const FMappingContextInfo& MappingContextToRestore)
		{
			return StatesToRestore.Contains(MappingContextToRestore.InputMappingContextState); 
		});
	}

	for (auto MappingContextToRestore : LocalMappingContextsToRestore)
	{
		if(!MappingContextToRestore.InputMappingContext) continue;
		SetMappingContext(MappingContextToRestore);
	}
}

void UGameEnhancedInputComponent::RemoveAllMappingContexts(const bool bIsSaveDefaultMappings, const bool bIsCacheRemovedMappings)
{
	if(AddedMappingContexts.IsEmpty()) return;
	const auto LocalAddedMappingContexts = AddedMappingContexts;
	for(const auto MappingInputMapping : LocalAddedMappingContexts)
	{
		if(!MappingInputMapping.InputMappingContext ||
			(MappingInputMapping.InputMappingContextState == EMappingContextState::Default && bIsSaveDefaultMappings)) continue;
		SetMappingContext(MappingInputMapping, false, bIsCacheRemovedMappings);
	}
}

void UGameEnhancedInputComponent::ResetToMainMappingContexts(const TArray<EMappingContextState>& StatesToReset,
	const bool bIsCacheRemovedMappings)
{
	if(!AddedMappingContexts.IsEmpty())
	{
		// Get All Added Mapping Contexts to remove
		TArray<FMappingContextInfo> LocalDifferenceMappingArray = AddedMappingContexts.FilterByPredicate([&](const FMappingContextInfo& AddedMappingContext)
		{
			const bool bLocalIsNotContainState = !StatesToReset.IsEmpty() && !StatesToReset.Contains(AddedMappingContext.InputMappingContextState);
			return !MainMappingContexts.Contains(AddedMappingContext) || bLocalIsNotContainState;
		});

		// Remove getted Mapping Contexts
		for(const auto AddedMappingContext : LocalDifferenceMappingArray)
		{
			if(!AddedMappingContext.InputMappingContext) continue;
			SetMappingContext(AddedMappingContext, false, bIsCacheRemovedMappings);
		}
	}
	
	// Add Main Mapping Contexts by state
	const auto LocalMainMappingContexts = MainMappingContexts;
	for(const auto MainMappingContext : LocalMainMappingContexts)
	{
		const bool bLocalIsNotContainState = !StatesToReset.IsEmpty() && !StatesToReset.Contains(MainMappingContext.InputMappingContextState);
		if(!MainMappingContext.InputMappingContext || AddedMappingContexts.Contains(MainMappingContext)|| bLocalIsNotContainState) continue;
		SetMappingContext(MainMappingContext);
	}
}

void UGameEnhancedInputComponent::RemoveMappingContextsByState(const TArray<EMappingContextState>& StatesToRemove, const bool bIsCacheRemovedMappings)
{
	if(AddedMappingContexts.IsEmpty() || StatesToRemove.IsEmpty()) return;
	TArray<FMappingContextInfo> LocalRemoveMappingArray = AddedMappingContexts.FilterByPredicate([&](const FMappingContextInfo& AddedMappingContext)
	{
		return StatesToRemove.Contains(AddedMappingContext.InputMappingContextState);
	});

	// Remove Mapping Contexts by state
	for (const auto RemoveMappingContext : LocalRemoveMappingArray)
	{
		if(!RemoveMappingContext.InputMappingContext) continue;
		SetMappingContext(RemoveMappingContext,false, bIsCacheRemovedMappings);
	}
}

void UGameEnhancedInputComponent::RemoveMainMappingContexts(const bool bIsCacheRemovedMappings,
	const FModifyContextOptions& Options)
{
	if(MainMappingContexts.IsEmpty()) return;
	const auto LocalMainMappingContexts = MainMappingContexts;
	for (const auto MainMappingContext : LocalMainMappingContexts)
	{
		if(!MainMappingContext.InputMappingContext) continue;
		SetMappingContext(MainMappingContext, false, bIsCacheRemovedMappings, Options);
	}
}

void UGameEnhancedInputComponent::SetMainMappingContexts(const UDA_MainMappingContexts* MainMappingContextsDA,
                                                         const FReplaceMainMappingContextsOptions& ReplaceMainMappingContextsOptions)
{
	if(!MainMappingContextsDA) return;
	// Remove Current Main Mappings
	if (ReplaceMainMappingContextsOptions.bIsRemoveCurrentMainMappingContexts)
		RemoveMainMappingContexts(ReplaceMainMappingContextsOptions.bIsCacheRemovedMappingContexts);
	MainMappingContexts = MainMappingContextsDA->GetMainMappingContexts();
	// Reset to Main Mapping Contexts
	if(ReplaceMainMappingContextsOptions.bIsResetToMainMappingContexts)
	{
		ResetToMainMappingContexts({}, ReplaceMainMappingContextsOptions.bIsCacheRemovedMappingContexts);
		return;
	}
	// Add To Current Main Mapping Contexts
	if(ReplaceMainMappingContextsOptions.bIsAddToCurrentMappingContexts)
	{
		const auto LocalMainMappingContexts = MainMappingContexts;
		for (const auto MainMappingContext : LocalMainMappingContexts)
		{
			if(!MainMappingContext.InputMappingContext) continue;
			SetMappingContext(MainMappingContext);
		}
	}
}

// Getter

const TArray<UInputMappingContext*> UGameEnhancedInputComponent::GetMappingContextsByState(const EMappingContextState FindByState) const
{
	const auto LocalMappingContextsInfoByState = GetMappingContextsInfoByState(FindByState);

	if(LocalMappingContextsInfoByState.IsEmpty()) return {};

	TArray<UInputMappingContext*> OutArrayOfFindedMappingContexts;
	for (const auto MappingContextsInfoByState : LocalMappingContextsInfoByState)
	{
		if(!MappingContextsInfoByState.InputMappingContext) continue;
		OutArrayOfFindedMappingContexts.AddUnique(MappingContextsInfoByState.InputMappingContext);
	}

	return MoveTemp(OutArrayOfFindedMappingContexts);
}

const TArray<FMappingContextInfo> UGameEnhancedInputComponent::GetMappingContextsInfoByState(
	const EMappingContextState FindByState) const
{
	// Filter array by state
	return AddedMappingContexts.FilterByPredicate([&](const FMappingContextInfo& AddedMappingContext)
	{
		return AddedMappingContext.InputMappingContextState == FindByState;
	});
}


// Debug

const FString UGameEnhancedInputComponent::GetDebugMappingContextState(const EMappingContextState MappingContextState) const
{
	FString LocalStateString;
	switch (MappingContextState)
	{
		case EMappingContextState::Default:
			LocalStateString = "Default";
			break;
		case EMappingContextState::Main:
			LocalStateString = "Main";
			break;
		case EMappingContextState::Secondary:
			LocalStateString = "Secondary";
			break;
		default:
			LocalStateString = "No mapping state";
	}

	return LocalStateString;
}

void UGameEnhancedInputComponent::ShowDebugInfo()
{
	if(!bShowDebug) return;

	// Show Append Mapping Context
	uint32 IntCount = 1;
	auto LocalAddedMappingContexts = AddedMappingContexts;
	LocalAddedMappingContexts.Sort([](const FMappingContextInfo& LocalAddedMappingContextA, const FMappingContextInfo& LocalAddedMappingContextB)
	{
		return LocalAddedMappingContextA.InputPriority > LocalAddedMappingContextB.InputPriority;
	});
	
	for (const auto AddedInputMapping : LocalAddedMappingContexts)
	{
		if(!AddedInputMapping.InputMappingContext) continue;
		auto LocalStateString = FString::Printf(TEXT("%i) %s : %s - Priority : %i"), IntCount++,
			*GetDebugMappingContextState(AddedInputMapping.InputMappingContextState), *AddedInputMapping.InputMappingContext->GetName(),
			AddedInputMapping.InputPriority);
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, LocalStateString);
	}
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, "\nAdded Mapping Contexts: ");
	
	// Show Cached Mapping Context
	IntCount = 1;
	auto LocalCachedMappingContexts = CachedMappingContexts;
	LocalCachedMappingContexts.Sort([](const FMappingContextInfo& LocalCachedMappingContextA, const FMappingContextInfo& LocalCachedMappingContextB)
	{
		return LocalCachedMappingContextA.InputPriority > LocalCachedMappingContextB.InputPriority;
	});
	
	for (const auto CachedMappingContext : CachedMappingContexts)
	{
		if(!CachedMappingContext.InputMappingContext) continue;
		auto LocalStateString = FString::Printf(TEXT("%i) %s : %s - Priority : %i"), IntCount++,
			*GetDebugMappingContextState(CachedMappingContext.InputMappingContextState), *CachedMappingContext.InputMappingContext->GetName(),
			CachedMappingContext.InputPriority);
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, LocalStateString);
	}
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, "\nCached Mapping Contexts: ");

	// Show Current Main Mapping Contexts
	IntCount = 1;
	auto LocalMainMappingContexts = MainMappingContexts;
	LocalMainMappingContexts.Sort([](const FMappingContextInfo& LocalMainMappingContextA, const FMappingContextInfo& LocalMainMappingContextB)
	{
		return LocalMainMappingContextA.InputPriority > LocalMainMappingContextB.InputPriority;
	});
	
	for (const auto MainMappingContext : LocalMainMappingContexts)
	{
		if(!MainMappingContext.InputMappingContext) continue;
		auto LocalStateString = FString::Printf(TEXT("%i) %s : %s - Priority : %i"), IntCount++,
			*GetDebugMappingContextState(MainMappingContext.InputMappingContextState), *MainMappingContext.InputMappingContext->GetName(),
			MainMappingContext.InputPriority);
		GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, LocalStateString);
	}
	if(CurrentMainMappingContextsDA) GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, GetNameSafe(CurrentMainMappingContextsDA));
	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::White, "\nMain Mapping Contexts: ");
}
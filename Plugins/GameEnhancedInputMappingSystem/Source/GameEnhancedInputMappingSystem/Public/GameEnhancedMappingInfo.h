#pragma once

#include "InputMappingContext.h"
#include "GameEnhancedMappingInfo.generated.h"

UENUM(BlueprintType)
enum class EMappingContextState : uint8
{
	Default, // Default mapping: does not depend on the pawn - (pause, tab and etc)
	Main, // Main Mapping for character: movement, look, interact and etc
	Secondary // Secondary Mapping: key mapping in items, puzzles and in other actors
};

UENUM(BlueprintType)
enum class EPlayerInputMode : uint8
{
	None UMETA(Hidden), 
	Game,
	UI,
	GameAndUI
};

USTRUCT(BlueprintType)
struct FMappingContextInfo
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		TObjectPtr<UInputMappingContext> InputMappingContext = nullptr;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		EMappingContextState InputMappingContextState = EMappingContextState::Default;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		int32 InputPriority = 0;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		FName MappingContextName = "";

	bool operator==(const FMappingContextInfo& OtherMappingContextInfo) const
	{
		return OtherMappingContextInfo.InputMappingContext == InputMappingContext &&
			OtherMappingContextInfo.InputMappingContext != nullptr
		&& InputMappingContext != nullptr;
	}
	
	FMappingContextInfo() = default;

	FMappingContextInfo(TObjectPtr<UInputMappingContext> NewInputMappingContext, const EMappingContextState NewInputMappingContextState = EMappingContextState::Default,
		const int32 NewInputPriority = 0, const FName NewMappingContextName = "") : InputMappingContext(NewInputMappingContext), InputMappingContextState(NewInputMappingContextState),
	InputPriority(NewInputPriority), MappingContextName(NewMappingContextName) {}
	
};

USTRUCT(BlueprintType)
struct FReplaceMainMappingContextsOptions
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		bool bIsResetToMainMappingContexts = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		bool bIsAddToCurrentMappingContexts = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		bool bIsRemoveCurrentMainMappingContexts = false;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="MappingContextInfo")
		bool bIsCacheRemovedMappingContexts = false;
	
	FReplaceMainMappingContextsOptions(const bool bNewIsResetToMainMappingContexts = false, const bool bNewIsAddToCurrentMappingContexts = false,
		const bool bNewIsRemoveCurrentMainMappingContexts = false, const bool bNewIsCacheRemovedMappingContexts = false) :
	bIsResetToMainMappingContexts(bNewIsResetToMainMappingContexts), bIsAddToCurrentMappingContexts(bNewIsAddToCurrentMappingContexts),
	bIsRemoveCurrentMainMappingContexts(bNewIsRemoveCurrentMainMappingContexts), bIsCacheRemovedMappingContexts(bNewIsCacheRemovedMappingContexts) {}
	
};
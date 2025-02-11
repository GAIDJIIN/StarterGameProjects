// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystemInterface.h"
#include "GameEnhancedInputComponent.generated.h"

struct FReplaceMainMappingContextsOptions;
class UDA_MainMappingContexts;
struct FModifyContextOptions;
struct FMappingContextInfo;
class UEnhancedInputLocalPlayerSubsystem;
class UInputMappingContext;

/*
 * Delegates Manipulate with Mapping Contexts
 */

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnAddMappingContext, UInputMappingContext*, AddedInputMappingContext);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRemoveMappingContext, UInputMappingContext*, RemovedInputMappingContext);

/*
 * Game Enhanced Input Component for manipulate mapping contexts in game
 * Prefer attach only in Player Controller
*/

UCLASS(ClassGroup=(Input), meta=(BlueprintSpawnableComponent))
class GAMEENHANCEDINPUTMAPPINGSYSTEM_API UGameEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:

	// Overrides
	
	UGameEnhancedInputComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void BeginPlay() override;
	
	// Functions
	// Setter

	// Main Method To Remove Mappings. Cache mappings only if bIsAdd == false
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo", meta=(AdvancedDisplay="bIsCacheRemovedMapping"))
		const bool SetMappingContext(
			const FMappingContextInfo& MappingContextInfo,
			const bool bIsAdd = true,
			const bool bIsCacheRemovedMapping = false,
			const FModifyContextOptions& Options = FModifyContextOptions());
	// Restore Cached Mapping Contexts if possible. If StatesToRestore.IsEmpty() == true - restore all
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo", meta=(AutoCreateRefTerm="StatesToRestore"))
		void RestoreCachedMappingContexts(const TArray<EMappingContextState>& StatesToRestore);
	// Reset All Main Mapping Contexts by state to default and remove others. If StatesToReset.IsEmpty() == true - reset all main mappings
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo", meta=(AutoCreateRefTerm="StatesToReset"))
		void ResetToMainMappingContexts(const TArray<EMappingContextState>& StatesToReset, const bool bIsCacheRemovedMappings = false);
	// Remove Mapping Contexts by States
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo", meta=(AutoCreateRefTerm="StatesToRemove"))
		void RemoveMappingContextsByState(const TArray<EMappingContextState>& StatesToRemove, const bool bIsCacheRemovedMappings = false);
	// Remove All Added Mapping Contexts. If bIsSaveDefaultMappings = true - dont remove Default Mapping Contexts
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		void RemoveAllMappingContexts(const bool bIsSaveDefaultMappings = true, const bool bIsCacheRemovedMappings = false);
	// Remove All Cached Mapping Contexts
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		FORCEINLINE void RemoveAllCachedMappingContexts() { CachedMappingContexts.Empty(); }
	/*
	 *Remove current Main Mapping Contexts. If change Main Mapping Contexts and dont remove last Main Mapping Contexts -
	 *remove ONLY CURRENT MAIN MAPPING CONTEXTS
	*/
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo", meta=(AutoCreateRefTerm="Options"))
		void RemoveMainMappingContexts(const bool bIsCacheRemovedMappings = false, const FModifyContextOptions& Options = FModifyContextOptions());
	// Set Main Mapping Contexts
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		void SetMainMappingContexts(
			const UDA_MainMappingContexts* MainMappingContextsDA,
			const FReplaceMainMappingContextsOptions& ReplaceMainMappingContextsOptions);
	
	// Getter

	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		const TArray<UInputMappingContext*> GetMappingContextsByState(const EMappingContextState FindByState) const;
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		const TArray<FMappingContextInfo> GetMappingContextsInfoByState(const EMappingContextState FindByState) const;
	UFUNCTION(BlueprintCallable, Category="MappingContextInfo")
		const TArray<FMappingContextInfo>& GetAllAddedMappingContexts() const { return AddedMappingContexts; }

	// Delegates

	UPROPERTY(BlueprintAssignable, Category="MappingContextInfo|Delegates")
		FOnRemoveMappingContext OnRemoveMappingContext;
	UPROPERTY(BlueprintAssignable, Category="MappingContextInfo|Delegates")
		FOnAddMappingContext OnAddMappingContext;
	
private:

	// Variables
	// BP

	// Current Main Mapping Contexts. Can be changed in runtime
	UPROPERTY(EditAnywhere, Category="MappingContextInfo", meta=(AllowPrivateAccess))
		TObjectPtr<UDA_MainMappingContexts> CurrentMainMappingContextsDA;
	UPROPERTY(EditAnywhere, Category="MappingContextInfo", meta=(AllowPrivateAccess))
		bool bInitializeMainMappingContextsOnBeginPlay = true;
	UPROPERTY(EditAnywhere, Category="MappingContextInfo|Debug", meta=(AllowPrivateAccess))
		bool bShowDebug = false;

	// CPP

	/*
	 * Return Mapping Contexts when change Mapping Contexts (return from minigame to car drive or other inputs)
	 * Maybe create layers of mapping contexts in future ( TMap<uint32,TArray<FMappingContextInfo>>, layer_priority : mapping_context_info ) 
	*/
	TArray<FMappingContextInfo> CachedMappingContexts = {};
	// Current Added Mapping Contexts
	TArray<FMappingContextInfo> AddedMappingContexts = {};
	// Current Main Mapping Contexts
	TArray<FMappingContextInfo> MainMappingContexts = {};
	TWeakObjectPtr<UEnhancedInputLocalPlayerSubsystem> EnhancedInputLocalPlayerSubsystem = nullptr;
	
	// Functions
	// Setter
	
	// Getter
	
	FORCEINLINE const bool GetCanWorkWithEnhancedInputSubsystem() const
	{
		return EnhancedInputLocalPlayerSubsystem.IsValid() && EnhancedInputLocalPlayerSubsystem.Get();
	}
	
	// Debug

	const FString GetDebugMappingContextState(const EMappingContextState MappingContextState) const;

	#if !UE_BUILD_SHIPPING
		void ShowDebugInfo();
	#endif
};
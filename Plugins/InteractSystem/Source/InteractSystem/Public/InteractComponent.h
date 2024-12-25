//Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "InteractComponent.generated.h"

class UInteractWidgetManager;
class UWidgetComponent;

UENUM(BlueprintType)
enum class ECanInteractState : uint8
{
    None = 0 UMETA(Hidden),
    CanNotInteract = 1 << 0,
    UsedNow = 1 << 1,
    CanInteract = 1 << 2
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSuccessStartInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFindInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLostInteract);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnChangeCanInteractState, ECanInteractState, CanInteractState);



class UStatusesComponent;
class IInteractInterface;

UCLASS( Blueprintable, ClassGroup=(InteractSystem), meta=(BlueprintSpawnableComponent) )
class INTERACTSYSTEM_API UInteractComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UInteractComponent();

protected:
	virtual void BeginPlay() override;

public:
    
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Main Logic
    
    UFUNCTION(BlueprintCallable,Category="Interact Component")
		void StartInteract();
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        void StopInteract();
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        void InteractCheck();
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        void Interact();

    // Setter
    
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        void SetStopInteractCheck(const bool bIsInteractCheck);
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        void SetUseActorEyes(bool bNewbIsUseActorEyes) { bIsUseActorEyes = bNewbIsUseActorEyes; }
    
    // Getter

    UFUNCTION(BlueprintCallable,Category="Interact Component")
        FORCEINLINE bool GetIsValidInteractObject() const { return InteractableObject.IsValid() && InteractableObject.Get(); } 
    UFUNCTION(BlueprintCallable,Category="Interact Component")
        const void GetInteractItemWidgetLocation(FVector& WidgetLocation) const;
    UFUNCTION(BlueprintCallable, Category="Interact Component")
        const ECanInteractState GetCurrentInputState() const;
    
private:
    
    // Variables

    // If true - use start point camera location
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(AllowPrivateAccess))
        bool bIsUseActorEyes = false;
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(AllowPrivateAccess))
        bool bStopInteractCheck = false;
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(AllowPrivateAccess))
        float MaxInteractionDistance = 1000.f;
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(AllowPrivateAccess))
        float MaxInteractionRadius = 20.f;
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(ClampMin="0.01", AllowPrivateAccess))
        float InteractionFrequency = 0.1f;
    UPROPERTY(EditAnywhere, Category="Interact Setup", meta=(AllowPrivateAccess))
        TEnumAsByte<ECollisionChannel> InteractChannel = ECC_Visibility;
    UPROPERTY(EditAnywhere,  Category="Debug", meta=(AllowPrivateAccess))
        bool bShowInteractDebug = false;
    // Show Widget Comp location by sphere
    UPROPERTY(EditAnywhere, Category="Debug",meta=(AllowPrivateAccess, EditConditionHides, EditCondition="bShowInteractDebug"))
        bool bShowWidgetLocSphere = false;
    UPROPERTY(EditAnywhere, Category="Statuses", meta=(AllowPrivateAccess))
        FGameplayTag InteractNowTag;
    UPROPERTY(EditAnywhere, Category="Statuses", meta=(AllowPrivateAccess))
        FGameplayTagContainer CantInteractTag;

    // CPP
    
    TWeakObjectPtr<UStatusesComponent> StatusesComp;
    bool bStopNow = false;
    FTimerHandle CheckInteractTraceHandle;
    ECanInteractState CurrentCanInteractState = ECanInteractState::None;
    
    // Widget Info
    
    TObjectPtr<UWidgetComponent> InteractWidgetComp = nullptr;
    
    // Current Interact Info
    // Object for interact now (actor or actor comp)
    TWeakObjectPtr<UObject> InteractableObject;
    FHitResult CurrentHitResult;
    
    // Functions
    
    void FirstInitializeComp();
    
    // Interact

    bool TraceFromCamera(FHitResult& OutHit);
    void SetInteractCheck(const bool bIsCheckInteract);
    
    // Clear interact info
    void ClearCurrentInteractInfo();
    
    // Check can interact state
    const void CheckCanInteractState();
    void CancelInteractFromItem();
    UActorComponent* GetInteractComponent(const AActor* InteractActor) const;
    // Call delegates or find or lost object
    void OnFindOrLostInteractObject(const bool bIsFindInteract) const;
    
    // Debug
    
    void ShowDebug();
    const FString GetCurrentCanInteractStateStringDebug() const;
    
    // Delegates
    
    UPROPERTY(BlueprintAssignable)
        FOnSuccessStartInteract OnSuccessStartInteract;
    UPROPERTY(BlueprintAssignable)
        FOnFindInteract OnFindInteract;
    UPROPERTY(BlueprintAssignable)
        FOnLostInteract OnLostInteract;
    UPROPERTY(BlueprintAssignable)
        FOnChangeCanInteractState OnChangeCanInteractState;

    // Widget manager for manipulate of interact widgets
    TObjectPtr<UInteractWidgetManager> InteractWidgetManager = nullptr;
};
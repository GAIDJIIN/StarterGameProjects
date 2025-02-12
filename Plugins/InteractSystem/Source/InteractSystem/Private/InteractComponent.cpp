//Florist Game. All rights reserved.


#include "InteractComponent.h"
#include "StatusesComponent.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/InteractInterface.h"
#include "Kismet/KismetSystemLibrary.h"
#include "InteractWidgetManager/InteractWidgetManager.h"
#include "Interfaces/InteractWidgetInfoInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

UInteractComponent::UInteractComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

    if(GetNetMode() == NM_Standalone ||
        (GetNetMode() == NM_Client && GetOwnerRole() == ROLE_AutonomousProxy) ||
        (GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy && GetOwnerRole() == ROLE_Authority))
    {
        InteractWidgetComp = CreateDefaultSubobject<UWidgetComponent>("WidgetComponent");

        InteractWidgetManager = NewObject<UInteractWidgetManager>(this, UInteractWidgetManager::StaticClass(),"InteractWidgetManager");
    }
}

// First Initialize needed info
void UInteractComponent::FirstInitializeComp()
{
    if(!GetOwner() || !GetWorld()) return;
    StatusesComp = GetOwner()->FindComponentByClass<UStatusesComponent>();

    checkf(InteractWidgetComp && InteractWidgetManager, TEXT("No set Interact Widget Comp or Interact Widget Manager"))

    InteractWidgetManager->SetupInteractWidgetComp(InteractWidgetComp, GetOwner());

    SetInteractCheck(true);
}

void UInteractComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(false);
    FirstInitializeComp();
    SetComponentTickEnabled(true);
}

void UInteractComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	ShowDebug();
}


bool UInteractComponent::Server_StartInteract_Validate()
{
    return true;
}

bool UInteractComponent::Server_StopInteract_Validate()
{
    return true;
}


// Interact Logic
void UInteractComponent::Server_Interact_Implementation()
{
    if(!StatusesComp.IsValid()) return;
    bool bInteractNow = StatusesComp->GetIsContainStatus(FGameplayTag(InteractNowTag),true);
    bInteractNow ?  Server_StopInteract() : Server_StartInteract();
}

void UInteractComponent::Server_StartInteract_Implementation()
{
    if(!InteractableObject.IsValid()) return;
    if(StatusesComp.IsValid())
    {
        if(StatusesComp->GetIsContainStatuses(CantInteractTag,false,true))
        {
            IInteractInterface::Execute_FailedTryInteract(InteractableObject.Get(),CurrentHitResult);
            return;
        }
    }

    /*
     * Check is can interact with Interactable Object.
     * If cant interact with Interactable Object - return
     */
    if(!IInteractInterface::Execute_CanInteract(InteractableObject.Get(),CurrentHitResult,GetOwner()))
    {
        if(bShowInteractDebug) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Red,
            FString("Cant Interact With Object"));

        IInteractInterface::Execute_FailedTryInteract(InteractableObject.Get(),CurrentHitResult);
        return;
    }
    
	bool LocalInteractSuccess = IInteractInterface::Execute_StartInteract(InteractableObject.Get(),CurrentHitResult, GetOwner());
    if(LocalInteractSuccess)
    {
        if(bShowInteractDebug) GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Green,
            FString("Interact With Object!!!"));
        
        OnSuccessStartInteract.Broadcast();
    }
}

void UInteractComponent::Server_StopInteract_Implementation()
{
    if(!InteractableObject.IsValid()) return;
    IInteractInterface::Execute_StopInteract(InteractableObject.Get(), CurrentHitResult);
}

void UInteractComponent::Server_InteractCheck_Implementation()
{
    // Check is can interact now
    const bool bLocalIsValidInteractItem = InteractableObject.Get() && InteractableObject.IsValid();
    bool bCantCheckNow = false;
    bool bInteractNow = false;
    if(StatusesComp.IsValid())
    {
        bCantCheckNow = StatusesComp->GetIsContainStatuses(CantInteractTag,false,true);
        bInteractNow = StatusesComp->GetIsContainStatus(FGameplayTag(InteractNowTag),true);
    }
    
    bool bStopInteract = bStopInteractCheck || bInteractNow || bCantCheckNow;
    
    if (bStopNow)
    {
        // Return if stop now
        if(bStopInteract) return;
        bStopNow = false;
    }

    // Cancel from interact if cant interact by statuses info or if interact check was stopped
    if(bStopInteract)
    {
        if(!bLocalIsValidInteractItem) return;
        CancelInteractFromItem();
        if(bStopInteractCheck || bCantCheckNow) ClearCurrentInteractInfo();
        bStopNow = true;
        return;
    }

    // Try find interact object
    FHitResult LocalOutHit;
    bool LocalIsHit = TraceFromCamera(LocalOutHit);
    CurrentHitResult = LocalOutHit;
    
    // Cancel from interact if no hit
    if(!LocalIsHit)
    {
        if(!bLocalIsValidInteractItem) return;
        CancelInteractFromItem();
        ClearCurrentInteractInfo();
        return;
    }

    // Find Interactable Object
    UObject* LocalInteractableItem = nullptr;
    if(LocalOutHit.GetActor()) LocalInteractableItem = GetInteractComponent(LocalOutHit.GetActor());
    
    // Cancel if new interact object is different as previous
    if(bLocalIsValidInteractItem && InteractableObject.Get() != LocalInteractableItem)
    {
        CancelInteractFromItem();
    }

    // Clear Interactable Object if current finded object is empty
    if(!LocalInteractableItem)
    {
        ClearCurrentInteractInfo();
        return;
    }
    
    // Save if finded Interact Object is new
    const bool bLocalIsFindedObjectNew = LocalInteractableItem != InteractableObject.Get();
    
    // Save Interact Object
    InteractableObject = LocalInteractableItem;
    
    // Execute Trace Hit. Call after find valid InteractableObject
    IInteractInterface::Execute_TraceHit(InteractableObject.Get(), CurrentHitResult, GetOwner());

    // Show or Hide widget
    const auto LocalIsShowInteractWidget = IInteractWidgetInfoInterface::Execute_GetIsShowInteractWidget(LocalInteractableItem);

    // Setup interact widget by interact widget manager
    TSubclassOf<UUserWidget> LocalInteractWidgetRef;
    UObject* LocalInteractWidgetOwnerObject;
    IInteractWidgetInfoInterface::Execute_GetInteractWidget(InteractableObject.Get(), LocalInteractWidgetRef, LocalInteractWidgetOwnerObject);

    auto LocalPlayerController = UGameplayStatics::GetPlayerController(GetWorld(),0);
    if(InteractWidgetManager && LocalPlayerController)
    {
        InteractWidgetManager->ToggleVisibilityWidget(LocalPlayerController, LocalInteractWidgetRef, LocalInteractWidgetOwnerObject,
        GetOwner(), LocalIsShowInteractWidget);
    }
    
    // Set Interact Widget Location if widget comp is valid and InteractWidgetLocation != ComponentLocation()
    FVector LocalInteractItemWidgetLocation;
    GetInteractItemWidgetLocation(LocalInteractItemWidgetLocation);
    if(LocalIsShowInteractWidget && InteractWidgetComp && LocalInteractItemWidgetLocation != InteractWidgetComp->GetComponentLocation())
        InteractWidgetComp->SetWorldLocation(LocalInteractItemWidgetLocation);

    // Call is find new interact object
    if(bLocalIsFindedObjectNew) OnFindOrLostInteractObject(true);
    
    // Check Can Interact State - call in end after all calculations
    CheckCanInteractState();
}

void UInteractComponent::OnFindOrLostInteractObject(const bool bIsFindInteract) const
{
    bIsFindInteract ? OnFindInteract.Broadcast() : OnLostInteract.Broadcast();
}

bool UInteractComponent::Server_InteractCheck_Validate()
{
    return true;
}

bool UInteractComponent::Server_Interact_Validate()
{
    return true;
}

void UInteractComponent::SetStopInteractCheck(const bool bIsInteractCheck)
{
    bStopInteractCheck = bIsInteractCheck;
    if(bStopInteractCheck)
    {
        if(!InteractableObject.IsValid()) return;
        CancelInteractFromItem();
        CurrentHitResult = FHitResult();
        InteractableObject = nullptr;
    }
}

void UInteractComponent::CancelInteractFromItem()
{
    if(!InteractableObject.IsValid()) return;
    IInteractInterface::Execute_CancelCanInteract(InteractableObject.Get(), CurrentHitResult);
    if(InteractWidgetManager) InteractWidgetManager->HideVisibilityWidget();
    OnFindOrLostInteractObject(false);
}

// Trace from camera
bool UInteractComponent::TraceFromCamera(FHitResult& OutHit)
{
    auto LocalController = UGameplayStatics::GetPlayerController(GetWorld(),0);
    if(!LocalController || !LocalController->PlayerCameraManager || !GetWorld()) return false;

    // Calculate Start End loc
    FVector PlayerViewLoc;
    FRotator PlayerViewRot;
    LocalController->GetPlayerViewPoint(PlayerViewLoc,PlayerViewRot);
    FVector ActorEyesLoc;
    FRotator ActorEyesRot;
    GetOwner()->GetActorEyesViewPoint(ActorEyesLoc,ActorEyesRot);
    const FVector Start = bIsUseActorEyes ? ActorEyesLoc : PlayerViewLoc;
    const FVector End = (bIsUseActorEyes ?
        UKismetMathLibrary::FindClosestPointOnLine(ActorEyesLoc,PlayerViewLoc,PlayerViewRot.Vector()) : PlayerViewLoc) + PlayerViewRot.Vector()*MaxInteractionDistance;
    
    const bool bHitTrace = UKismetSystemLibrary::SphereTraceSingle
    (
        GetWorld(),
        Start,
        End,
        MaxInteractionRadius,
        UEngineTypes::ConvertToTraceType(InteractChannel),
        false,
        {GetOwner()},
        bShowInteractDebug ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
        OutHit,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        InteractionFrequency
        );
    
    return bHitTrace;
}

void UInteractComponent::SetInteractCheck(const bool bIsCheckInteract)
{
    if(!GetWorld()) return;

    // Set timer to check trace
    if(bIsCheckInteract && !bStopInteractCheck)
    {
        if(!GetWorld()->GetTimerManager().IsTimerActive(CheckInteractTraceHandle))
            GetWorld()->GetTimerManager().SetTimer(CheckInteractTraceHandle, this,
                &UInteractComponent::Server_InteractCheck, InteractionFrequency, true);
    }
    // Clear timer if stop interact OR force stop interact
    else GetWorld()->GetTimerManager().ClearTimer(CheckInteractTraceHandle);
}

// Clear interact info
void UInteractComponent::ClearCurrentInteractInfo()
{
    InteractableObject = nullptr;
    CurrentHitResult = FHitResult();
    CurrentCanInteractState = ECanInteractState::None;
}

// Getter
const void UInteractComponent::GetInteractItemWidgetLocation(FVector& WidgetLocation) const
{
    if(!InteractableObject.IsValid()) WidgetLocation = FVector::Zero();
    IInteractWidgetInfoInterface::Execute_GetInteractWidgetLocation(InteractableObject.Get(), WidgetLocation);
}

const ECanInteractState UInteractComponent::GetCurrentInputState() const
{
    ECanInteractState LocalNewCanInteractState = CurrentCanInteractState;
        
    if(!InteractableObject.IsValid()) LocalNewCanInteractState = ECanInteractState::None;
        
    // else if( Is Used Now Condition ) LocalNewCanInteractState = ECanInteractState::UsedNow;
        
    else
    {
        const bool bLocalCanInteract = IInteractInterface::Execute_CanInteract(InteractableObject.Get(),CurrentHitResult,GetOwner());
        LocalNewCanInteractState = bLocalCanInteract ? ECanInteractState::CanInteract : ECanInteractState::CanNotInteract;
    }

    return MoveTemp(LocalNewCanInteractState);
}

const void UInteractComponent::CheckCanInteractState()
{
    ECanInteractState LocalNewCanInteractState = GetCurrentInputState();
    
    // Call Delegate if new state
    if(CurrentCanInteractState != LocalNewCanInteractState)
    {
        CurrentCanInteractState = LocalNewCanInteractState;
        OnChangeCanInteractState.Broadcast(CurrentCanInteractState);
    }
}

UActorComponent* UInteractComponent::GetInteractComponent(const AActor* InteractActor) const
{
    if(!InteractActor) return nullptr;
    const auto LocalInteractComponents = InteractActor->GetComponentsByInterface(UInteractInterface::StaticClass());
    if(LocalInteractComponents.IsEmpty()) return nullptr;
    
    for(const auto InteractComp : LocalInteractComponents)
    {
        if(InteractComp->Implements<UInteractInterface>()) return InteractComp;
    }
    
    return nullptr;
}


#if !UE_BUILD_SHIPPING
    // Debug show logic
    void UInteractComponent::ShowDebug()
    {
        if(!bShowInteractDebug) return;
        FString DebugText;
        if(InteractableObject.IsValid())
        {
            DebugText += "Interactable Object is: " + FString(InteractableObject.Get() ? "True" : "False");
            DebugText += "\nInteractable Name: " + InteractableObject->GetName();
            if(InteractWidgetComp && !InteractWidgetComp->bHiddenInGame)
            {
                UKismetSystemLibrary::DrawDebugString(
                    GetWorld(),
                    InteractWidgetComp->GetComponentLocation(),
                    GetNameSafe(InteractableObject.Get())
                    );
            }
        }

        DebugText += "\nInteract State: " + GetCurrentCanInteractStateStringDebug();
        DebugText += "\nIs stop now: " + FString(bStopNow ? "True" : "False");

        // Get debug info from Interact Widget Manager
        if(InteractWidgetManager) DebugText += "\n" + InteractWidgetManager->GetDebugInteractWidgetsString() + "\n";
        
        UKismetSystemLibrary::PrintString(GetWorld(),
               DebugText,
               true,
               false,
               FLinearColor::Red,
               0.0f);

        if(InteractWidgetComp && bShowWidgetLocSphere && InteractWidgetManager) UKismetSystemLibrary::DrawDebugSphere(GetOwner(),
            InteractWidgetComp->GetComponentLocation(),30.f,12.f, InteractWidgetManager->GetIsWidgetVisibleNow() ?
            FLinearColor::Green : FLinearColor::Red, 0.f, 2.f);
        
    }

    const FString UInteractComponent::GetCurrentCanInteractStateStringDebug() const
    {
        switch (CurrentCanInteractState)
        {
            case ECanInteractState::None:
                return "None";
            case ECanInteractState::CanInteract:
                return "CanInteract";
            case ECanInteractState::UsedNow:
                return "UsedNow";
            case ECanInteractState::CanNotInteract:
                return "CanNotInteract";
            default:
                return "Default";
        }
    }
#endif
// Florist Game. All rights reserved.


#include "DialogueSystem/DialogueComponent.h"
#include "AIController.h"
#include "BrainComponent.h"
#include "StatusesComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Camera/CameraComponent.h"
#include "Modifiers/CameraModifierFOV.h"
#include "Components/AudioComponent.h"
#include "DialogueSystem/DialogueWidget.h"
#include "Engine/StreamableManager.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

DEFINE_LOG_CATEGORY(DialogueComponent);

UDialogueComponent::UDialogueComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	DialogueAudioComponent = CreateDefaultSubobject<UAudioComponent>(TEXT("DialogueAudioComponent"));
	CameraModifierFOVClass = UCameraModifierFOV::StaticClass();
	if(GetOwner()) DialogueAudioComponent->SetupAttachment(GetOwner()->GetRootComponent());
}


//-----------------------------------------------Interact Interface--------------------------------------------------//
bool UDialogueComponent::StartInteractLogic_Implementation(FHitResult HitResult, AActor* InteractByActor)
{
	if(!InteractByActor) return false;
	const TObjectPtr<AActor> LocalDialogueInstigator = Cast<AActor>(InteractByActor);
	ShowDialogue(LocalDialogueInstigator);
	return true;
}

void UDialogueComponent::GetInteractWidgetLocationLogic_Implementation(FVector& WidgetLocation) const
{
	if(!GetOwner()) WidgetLocation = FVector::Zero();
	
	GetLookAtLocation(WidgetLocation);
}

bool UDialogueComponent::CanInteractLogic_Implementation(FHitResult HitResult, AActor* QueryFromActor)
{
	return true;
}
//-------------------------------------------------------------------------------------------------------------------//

void UDialogueComponent::BeginPlay()
{
	Super::BeginPlay();
	SetComponentTickEnabled(true);
	
	// Add Controller for Owner
	const auto LocalOwner = GetOwner();
	if(!LocalOwner || !GetWorld()) return;
	OwnerStatusComponent = Cast<UStatusesComponent>(LocalOwner->GetComponentByClass(UStatusesComponent::StaticClass())); 
	AIController = GetWorld()->SpawnActor<AAIController>(AAIController::StaticClass());
}

void UDialogueComponent::TickComponent(float DeltaTime, ELevelTick TickType,
	FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	SetLookAtSpeakerRotation(DeltaTime);
}

// Dialogue Logic
void UDialogueComponent::ShowDialogue(AActor* DialogueInstigator)
{
	if(!DialogueInstigator || !GetWorld() || !DialogueWidgetClass) return;
	
	const auto LocalInstigatorPawn = Cast<APawn>(DialogueInstigator);
	if(!LocalInstigatorPawn) return;
	
	const auto LocalPlayerController = LocalInstigatorPawn->GetController<APlayerController>();
	if(!LocalPlayerController) return;

	// Create Dialogue Widget
	if(DialogueWidget) DialogueWidget->RemoveFromParent();
	auto LocalDialogueWidget = CreateWidget(LocalPlayerController, DialogueWidgetClass);
	DialogueWidget = Cast<UDialogueWidget>(LocalDialogueWidget);
	if(DialogueWidget)
	{
		DialogueWidget->SetupDialogueWidget(this);
		DialogueWidget->AddToViewport(DialogueWidgetPriority);
	}
	
	// Set Instigator only if func not return
	CurrentDialogueInstigator = DialogueInstigator;
	
	
	OnExitFromDialogue.AddDynamic(this, &UDialogueComponent::HideDialogueLogic);
	
	FInputModeUIOnly LocalInputMode;
	LocalInputMode.SetWidgetToFocus(DialogueWidget->GetCachedWidget());
	LocalPlayerController->SetInputMode(LocalInputMode);
	LocalPlayerController->SetShowMouseCursor(true);

	// Setup Start Dialogue Member Name
	SetDialogueMemberName(OwnerName, false);
	SetDialogueMemberName(InstigatorName, true);
	
	// Start behavior tree
	if(AIController)
	{
		AIController->RunBehaviorTree(DialogueBehaviorTree);
		AIController->GetBlackboardComponent()->SetValueAsObject(DialogueCompBBNameValue,this);
	}

	// Add FOV Camera Modifier
	const auto LocalPlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0);
	if(LocalPlayerCameraManager)
	{
		const auto LocalCameraModifier = LocalPlayerCameraManager->AddNewCameraModifier(CameraModifierFOVClass);
		if(LocalCameraModifier)
		{
			CameraModifierFOV = Cast<UCameraModifierFOV>(LocalCameraModifier);
			if(CameraModifierFOV) CameraModifierFOV->SetIsActive(true);
		}
	}

	// Add Dialogue Instigator Start Status
	const auto LocalDialogueInstigatorComp = Cast<UStatusesComponent>(CurrentDialogueInstigator->GetComponentByClass(UStatusesComponent::StaticClass())); 
	if(LocalDialogueInstigatorComp) LocalDialogueInstigatorComp->AddConstantStatuses(FGameplayTagContainer(DialogueStartInstigatorStatus));

	// Add Dialogue Owner Start Status
	if(OwnerStatusComponent) OwnerStatusComponent->AddConstantStatuses(FGameplayTagContainer(DialogueStartOwnerStatus));
	
	bIsLookAtSpeakerNow = true;
	bIsInDialogueNow = true;
	OnStartDialogue.Broadcast();
}

void UDialogueComponent::HideDialogue()
{
	HideDialogueLogic();
	OnExitFromDialogue.Broadcast();
}

void UDialogueComponent::HideDialogueLogic()
{
	if(!CurrentDialogueInstigator) return;
	const auto LocalPlayerController = Cast<APawn>(CurrentDialogueInstigator)->GetController<APlayerController>();
	if(!LocalPlayerController) return;

	// Clear Dialogue Widget
	if(DialogueWidget)
	{
		DialogueWidget->RemoveFromParent();
		DialogueWidget = nullptr;
	}
	
	OnExitFromDialogue.RemoveDynamic(this, &UDialogueComponent::HideDialogueLogic);
	FInputModeGameOnly LocalInputMode; 
	LocalPlayerController->SetInputMode(LocalInputMode);
	LocalPlayerController->SetShowMouseCursor(false);
	
	// Stop behavior tree
	if(AIController)
	{
		const auto LocalBrainComponent = AIController->GetBrainComponent();
		if(LocalBrainComponent)
		{
			LocalBrainComponent->StopLogic("Stop dialogue");
			
			// Clear All Data from Blackboard Value
			auto LocalBlackboardComp = LocalBrainComponent->GetBlackboardComponent();
			if(LocalBlackboardComp)
			{
				auto LocalBlackboardData = LocalBlackboardComp->GetBlackboardAsset();
				if(LocalBlackboardData)
				{
					for(auto LocalBlackboardKey : LocalBlackboardData->GetKeys())
					{
						LocalBlackboardComp->ClearValue(LocalBlackboardKey.EntryName);
					}
				}
			}
		}
	}

	// Remove FOV Camera Modifier
	if(CameraModifierFOV) CameraModifierFOV->SetIsActive(false);

	// Remove Dialogue Instigator Start Status
	const auto LocalDialogueComp = Cast<UStatusesComponent>(CurrentDialogueInstigator->GetComponentByClass(UStatusesComponent::StaticClass())); 
	if(LocalDialogueComp) LocalDialogueComp->RemoveStatuses(FGameplayTagContainer(DialogueStartInstigatorStatus));

	// Remove Dialogue Owner Start Status
	if(OwnerStatusComponent) OwnerStatusComponent->RemoveStatuses(FGameplayTagContainer(DialogueStartOwnerStatus));
	
	CurrentDialogueInstigator = nullptr;
	bIsLookAtSpeakerNow = false;
	bIsInDialogueNow = false;
}

const bool UDialogueComponent::GetLookAtLocation(FVector& LookTargetLocation) const
{
	LookTargetLocation = FVector::Zero();
	if (!GetOwner()) return false; 

	// Set Look At Location
	
	switch (LookDialogueState)
	{
	case ELookDialogueState::None:
		return false;
	case ELookDialogueState::Socket:
		{
			const auto LocalOwner = GetOwner();
			TArray<USceneComponent*> LocalComponents;
			LocalOwner->GetComponents<USceneComponent>(LocalComponents,true);
			if(LocalComponents.IsEmpty()) return false;
			for (const auto LocalComponent : LocalComponents)
			{
				if(!LocalComponent || !LocalComponent->DoesSocketExist(SocketLookAt)) continue;
				LookTargetLocation = LocalComponent->GetSocketLocation(SocketLookAt);
			}
			break;
		}
	case ELookDialogueState::Location:
		{
			const auto LocalTransformedLocation = GetOwner()->GetActorTransform().TransformPosition(LookAtLocation);
			LookTargetLocation = LocalTransformedLocation;
			break;
		}
	case ELookDialogueState::OwnerLocation:
		{
			if(GetOwner()) LookTargetLocation = GetOwner()->GetActorLocation();
			break;
		}
	default:
		{
			if(GetOwner()) LookTargetLocation = GetOwner()->GetActorLocation();
			break;
		}
	}

	return true;
}

void UDialogueComponent::SetLookAtSpeakerRotation(const float DeltaSeconds)
{
	if(!bIsLookAtSpeakerNow || !CurrentDialogueInstigator) return;
	FVector LocalTargetLocation;
	FRotator LocalLookTargetRotation;
	FRotator LocalCalculatedRot;
	const bool bLocalIsCanLookAtRotation = GetLookAtLocation(LocalTargetLocation);
	if(!bLocalIsCanLookAtRotation)
	{
		bIsLookAtSpeakerNow = false;
		return;
	}
	
	// If use Camera for Look
	if(bIsUseCameraForLook)
	{
		// Try Find Camera component
		const auto LocalCameraComp = CurrentDialogueInstigator->GetComponentByClass<UCameraComponent>();
		if(!LocalCameraComp)
		{
			bIsLookAtSpeakerNow = false;
			return;
		}

		// Find look at target
		LocalLookTargetRotation = UKismetMathLibrary::FindLookAtRotation(LocalCameraComp->GetComponentLocation(),LocalTargetLocation);

		// Interpolate Control Rotation
		LocalCalculatedRot = UKismetMathLibrary::RInterpTo(LocalCameraComp->GetComponentRotation(), LocalLookTargetRotation, DeltaSeconds, LookAtSpeed);
		LocalCameraComp->SetWorldRotation(LocalCalculatedRot);
		if(UKismetMathLibrary::EqualEqual_RotatorRotator(LocalCameraComp->GetComponentRotation(),LocalLookTargetRotation, 0.1f))
		{ bIsLookAtSpeakerNow = false; }
	}
	
	// If use Control Rotation for Look
	else
	{
		const auto LocalPawn = Cast<APawn>(CurrentDialogueInstigator);
		if(!LocalPawn)
		{
			bIsLookAtSpeakerNow = false;
			return;
		}
		
		const auto LocalPlayer = LocalPawn->GetController<APlayerController>();
		if(!LocalPlayer)
		{
			bIsLookAtSpeakerNow = false;
			return;
		}

		// Find look at target
		FVector PlayerViewLoc;
		FRotator PlayerViewRot;
		LocalPlayer->GetPlayerViewPoint(PlayerViewLoc,PlayerViewRot);
		LocalLookTargetRotation = UKismetMathLibrary::FindLookAtRotation(PlayerViewLoc,LocalTargetLocation);

		// Interpolate Control Rotation
		LocalCalculatedRot = UKismetMathLibrary::RInterpTo(LocalPlayer->GetControlRotation(), LocalLookTargetRotation, DeltaSeconds, LookAtSpeed);
		LocalPlayer->SetControlRotation(LocalCalculatedRot);
		
		if(UKismetMathLibrary::EqualEqual_RotatorRotator(LocalPlayer->GetControlRotation(),LocalLookTargetRotation, 0.1f))
		{ bIsLookAtSpeakerNow = false; }
	}
}

// Try Async Load Sound
void UDialogueComponent::TryLoadAndPlaySound(
	TDelegate<TDelegate<void()>::RetValType(), FDefaultDelegateUserPolicy> OnLoadDelegate,
	const TSoftObjectPtr<USoundBase> PlaySound)
{
	FStreamableManager Streamable;
	Streamable.RequestAsyncLoad(PlaySound.ToSoftObjectPath(), OnLoadDelegate);
}

void UDialogueComponent::SpeakLogic()
{
	const auto LocalSound = CurrentSpeakInfo.VoiceSound.Get();
	const auto LocalVoiceText = CurrentSpeakInfo.VoiceText;
	
	PlayVoiceSound(LocalSound);

	CurrentDialogueState = EDialogueState::Speak;
	OnSpeakStarted.Broadcast(LocalVoiceText,LocalSound);

	// Log
	UE_LOG(DialogueComponent, Log, TEXT("Speak: %s"), *LocalVoiceText.ToString());
}

void UDialogueComponent::ReplyLogic()
{
	const auto LocalSound = CurrentReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceSound.Get();
	const auto LocalVoiceText = CurrentReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceText;
	const auto LocalReplyIndex = CurrentReplyInfo.ReplyIndex;
	
	PlayVoiceSound(LocalSound);
	
	CurrentDialogueState = EDialogueState::Reply;
	OnReplyStarted.Broadcast(LocalVoiceText, LocalReplyIndex, LocalSound);

	// Log
	UE_LOG(DialogueComponent, Log, TEXT("Reply: %s \n Reply Index: %i"), *LocalVoiceText.ToString(), LocalReplyIndex);
}

void UDialogueComponent::PlayVoiceSound(USoundBase* VoiceSound)
{
	if(!VoiceSound) return;

	DialogueAudioComponent->SetSound(VoiceSound);
	DialogueAudioComponent->Play();
}

void UDialogueComponent::StopVoiceSound(const bool bIsFadeOut, const float bFadeOutTime, const float FadeOutLevel, const EAudioFaderCurve FadeCurve)
{
	if(!DialogueAudioComponent->GetSound() || !DialogueAudioComponent->IsPlaying()) return;
	
	if(bIsFadeOut) DialogueAudioComponent->FadeOut(bFadeOutTime, FadeOutLevel, FadeCurve);
	else DialogueAudioComponent->Stop();
}

const bool UDialogueComponent::GetIsInDialogueNow() const
{
	if(!OwnerStatusComponent) return bIsInDialogueNow;
	return bIsInDialogueNow || OwnerStatusComponent->GetIsContainStatus(DialogueStartOwnerStatus);
}

void UDialogueComponent::Speak(const FVoiceInfo SpeakInfo)
{
	CurrentSpeakInfo = SpeakInfo;
	if(SpeakInfo.VoiceSound.Get() || !SpeakInfo.VoiceSound.IsValid()) SpeakLogic();
	else
	{
		auto LocalLoadSoundDelegate = FStreamableDelegate::CreateUObject(this, &UDialogueComponent::SpeakLogic);
		TryLoadAndPlaySound(LocalLoadSoundDelegate, SpeakInfo.VoiceSound);
	}
}

void UDialogueComponent::Reply(const FReplyInfo ReplyInfo)
{
	CurrentReplyInfo = ReplyInfo;
	if(ReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceSound.Get() || !ReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceSound.IsValid()) ReplyLogic();
	else
	{
		auto LocalLoadSoundDelegate = FStreamableDelegate::CreateUObject(this, &UDialogueComponent::ReplyLogic);
		TryLoadAndPlaySound(LocalLoadSoundDelegate, ReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceSound);
	}
}

// Reply Speak Logic
void UDialogueComponent::FinishSpeak()
{
	StopVoiceSound();
	CurrentSpeakInfo = FVoiceInfo();
	CurrentDialogueState = EDialogueState::None;
	OnSpeakFinished.Broadcast();
}

void UDialogueComponent::FinishReply(const int32 ReplyIndex)
{
	StopVoiceSound();
	CurrentReplyInfo = FReplyInfo();
	CurrentDialogueState = EDialogueState::None;
	OnReplyFinished.Broadcast(ReplyIndex);
}

// Widget Logic
void UDialogueComponent::ShowRepliesOnWidget(const TArray<FPlayVoiceInfo>& Replies)
{
	if(!DialogueWidget) return;
	DialogueWidget->SetupReplies(Replies);
}

void UDialogueComponent::SetDialogueMemberName(const FText& NewMemberName, const bool bIsDialogueInstigatorName)
{
	if(!DialogueWidget) return;
	if(bIsDialogueInstigatorName) InstigatorName = NewMemberName;
	else OwnerName = NewMemberName;
	
	OnChangeMemberName.Broadcast(OwnerName, bIsDialogueInstigatorName);
}

// Debug
void UDialogueComponent::ShowDebug()
{
	if(!GetIsShowDebug() || !GetWorld()) return;
	FString DebugText = "Debug From BPC_Dialogue\n";
	FString LocalNameInstigator = "None";
	if(CurrentDialogueInstigator)
	{
		LocalNameInstigator = GetNameSafe(CurrentDialogueInstigator);
	}
	FString LocalDialogueState = "None";
	switch (CurrentDialogueState)
	{
		case EDialogueState::None:
			LocalDialogueState = "None";
			break;
		case EDialogueState::Reply:
			LocalDialogueState = "Reply";
			break;
		case EDialogueState::Speak:
			LocalDialogueState = "Speak";
			break;
		default:
			break;
	}

	FVector LocalLookAtLoc;
	GetLookAtLocation(LocalLookAtLoc);
	
	DebugText += "\nDialogue Widget : " + FString(DialogueWidget ? "True" : "False");
	DebugText += "\nDialogue Instigator : " + LocalNameInstigator;
	DebugText += "\nDialogue State : " + LocalDialogueState;
	DebugText += "\nCurrent Speak Info : " + CurrentSpeakInfo.VoiceText.ToString();
	DebugText += "\nCurrent Reply Info : " + CurrentReplyInfo.ReplyVoiceInfo.VoiceInfo.VoiceText.ToString() + "\nReply Index : "
	+ FString::FromInt(CurrentReplyInfo.ReplyIndex);
	DebugText += "\nLook At Location : " + LocalLookAtLoc.ToString();
	DebugText += "\nTry Look At Location Now : " + FString(bIsLookAtSpeakerNow ? "True" : "False");

	const auto LocalBrainComponent = AIController->GetBrainComponent();
	if(LocalBrainComponent)
	{
		DebugText += "\nIs Active Dialogue Tree : " + FString(LocalBrainComponent->IsRunning() ? "True" : "False");
	}
	
	UKismetSystemLibrary::PrintString(GetWorld(),
			DebugText,
			true,
			true,
			FLinearColor(0.f,0.053782f, 1.f,1.f),
			0.f
			);
	
	UKismetSystemLibrary::DrawDebugSphere(GetWorld(),
			LocalLookAtLoc,
			10,
			12,
			FLinearColor::Red
			);
}
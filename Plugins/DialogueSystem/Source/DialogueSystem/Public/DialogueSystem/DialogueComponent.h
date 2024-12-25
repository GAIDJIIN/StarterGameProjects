// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueInfo.h"
#include "InteractActorComponent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "Components/AudioComponent.h"
#include "DialogueComponent.generated.h"


// Log

DECLARE_LOG_CATEGORY_EXTERN(DialogueComponent, Log, All);

// Finish Dialogue Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnStartDialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnExitFromDialogue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnSpeakFinished);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReplyFinished, int32, ReplyIndex);

// Dialogue Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnSpeakStarted, FText, SpeakText, USoundBase*, SpeakSound);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnReplyStarted, FText, ReplyText, const int32, ReplyIndex, USoundBase*, ReplySound);

// Dialogue Service Delegates

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnChangeMemberName, const FText&, MemberName, const bool, bIsDialogueInstigatorName);

// FWD Declarations

class UStatusesComponent;
class AAIController;
class UBehaviorTree;
class UDialogueWidget;
class UCameraModifierFOV;

UCLASS( ClassGroup=(Dialogue), meta=(BlueprintSpawnableComponent) )
class DIALOGUESYSTEM_API UDialogueComponent : public UInteractActorComponent
{
	GENERATED_BODY()

public:	
	UDialogueComponent();
	
	// Dialogue Logic
	
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void ShowDialogue(AActor* DialogueInstigator);
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void HideDialogue();

	// Setter
	
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void SetCurrentDialogueBehaviorTree(UBehaviorTree* NewDialogueBehaviorTree) { DialogueBehaviorTree = NewDialogueBehaviorTree; }
	
	// Getter
	
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		const EDialogueState GetCurrentDialogueState() const { return CurrentDialogueState; }
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		FVoiceInfo GetCurrentSpeakInfo() const { return CurrentSpeakInfo; }
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		FReplyInfo GetCurrentReplyInfo() const { return CurrentReplyInfo; }
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		AActor* GetDialogueInstigator() const { return CurrentDialogueInstigator; }
	UFUNCTION(BlueprintCallable, Category="Dialogue Widget")
		const FText& GetDialogueMemberName(const bool bIsDialogueInstigatorName) const
		{
			return bIsDialogueInstigatorName ? InstigatorName : OwnerName;
		};
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		const bool GetIsInDialogueNow() const;
	
	// Reply Speak Main Logic
	
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void Speak(const FVoiceInfo SpeakInfo);
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void Reply(const FReplyInfo ReplyInfo);
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void FinishSpeak();
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void FinishReply(const int32 ReplyIndex);

	// Widget Logic
	
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void ShowRepliesOnWidget(const TArray<FPlayVoiceInfo>& Replies);
	UFUNCTION(BlueprintCallable, Category="DialogueComponent|MainLogic")
		void SetDialogueMemberName(const FText& NewMemberName, const bool bIsDialogueInstigatorName = false);
	
protected:
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	// Interact Interface
	
	virtual bool CanInteractLogic_Implementation(FHitResult HitResult, AActor* QueryFromActor) override;
	virtual bool StartInteractLogic_Implementation(FHitResult HitResult, AActor* InteractByActor) override;
	virtual void GetInteractWidgetLocationLogic_Implementation(FVector& WidgetLocation) const override;
	
public:	
	// Finish Delegates
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnStartDialogue OnStartDialogue;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnExitFromDialogue OnExitFromDialogue;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnSpeakFinished OnSpeakFinished;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnReplyFinished OnReplyFinished;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnChangeMemberName OnChangeMemberName;
	
	// Dialogue Delegates
	
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnSpeakStarted OnSpeakStarted;
	UPROPERTY(BlueprintCallable, BlueprintAssignable, Category="DialogueComponent|Delegates")
		FOnReplyStarted OnReplyStarted;

	// Components
	UPROPERTY(BlueprintReadOnly, Category="DialogueComponent|Components")
		TObjectPtr<UAudioComponent> DialogueAudioComponent;

private:

	// Service Logic
	
	void TryLoadAndPlaySound(TDelegate<TDelegate<void()>::RetValType(), FDefaultDelegateUserPolicy> OnLoadDelegate,
		const TSoftObjectPtr<USoundBase> PlaySound);
	void SpeakLogic();
	void ReplyLogic();
	UFUNCTION()
		void HideDialogueLogic();

	const bool GetLookAtLocation(FVector& LookTargetLocation) const;
	void SetLookAtSpeakerRotation(const float DeltaSeconds);

	void PlayVoiceSound(USoundBase* VoiceSound);
	void StopVoiceSound(const bool bIsFadeOut = false, const float bFadeOutTime = 1.f, const float FadeOutLevel = 0.f, const EAudioFaderCurve FadeCurve = EAudioFaderCurve::Linear);
	
	// Debug
	
	virtual void ShowDebug() override;
	
	// Dialogue Setup Component
	
	UPROPERTY(EditAnywhere, Category="DialogueComponent",meta=(AllowPrivateAccess))
		FGameplayTag DialogueStartInstigatorStatus;
	UPROPERTY(EditAnywhere, Category="DialogueComponent",meta=(AllowPrivateAccess))
		FGameplayTag DialogueStartOwnerStatus;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|WidgetInfo",meta=(AllowPrivateAccess))
		int32 DialogueWidgetPriority = 0;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|WidgetInfo",meta=(AllowPrivateAccess))
		TSubclassOf<UDialogueWidget> DialogueWidgetClass = nullptr;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|AI",meta=(AllowPrivateAccess))
		TObjectPtr<UBehaviorTree> DialogueBehaviorTree = nullptr;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|AI",meta=(Tooltip="Blackboard name", AllowPrivateAccess))
		FName DialogueCompBBNameValue = "DialogueComp";
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess))
		ELookDialogueState LookDialogueState = ELookDialogueState::None;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess, EditCondition="LookDialogueState==ELookDialogueState::Socket", EditConditionHides))
		FName SocketLookAt = "None";
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess))
		TSubclassOf<UCameraModifierFOV> CameraModifierFOVClass;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess, MakeEditWidget = true, EditCondition="LookDialogueState==ELookDialogueState::Location", EditConditionHides))
		FVector LookAtLocation = FVector::Zero();
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess, ClampMin="0.1"))
		float LookAtSpeed = 5.f;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|LookAtInfo",meta=(AllowPrivateAccess))
		bool bIsUseCameraForLook = false;
	UPROPERTY(EditAnywhere, Category="DialogueComponent|MemberNameInfo",meta=(AllowPrivateAccess))
		FText OwnerName = FText::FromString("Stranger");	
	UPROPERTY(EditAnywhere, Category="DialogueComponent|MemberNameInfo",meta=(AllowPrivateAccess))
		FText InstigatorName = FText::FromString("Player");
	
	// Service
	
	TObjectPtr<AAIController> AIController = nullptr;
	TObjectPtr<UDialogueWidget> DialogueWidget = nullptr;
	EDialogueState CurrentDialogueState = EDialogueState::None;
	TObjectPtr<AActor> CurrentDialogueInstigator = nullptr;
	TObjectPtr<UCameraModifierFOV> CameraModifierFOV = nullptr;
	TObjectPtr<UStatusesComponent> OwnerStatusComponent = nullptr;
	bool bIsInDialogueNow = false;
	
	// Look At Speaker Now
	
	FRotator CurrentLookAtSpeakerRot = FRotator::ZeroRotator;
	bool bIsLookAtSpeakerNow = false;
	
	// Sound Info
	
	FVoiceInfo CurrentSpeakInfo;
	FReplyInfo CurrentReplyInfo;
	
};
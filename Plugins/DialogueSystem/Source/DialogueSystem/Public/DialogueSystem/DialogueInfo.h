// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueInfo.generated.h"


USTRUCT(BlueprintType)
struct FVoiceInfo
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		FText VoiceText;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		TSoftObjectPtr<USoundBase> VoiceSound;

	FVoiceInfo() {}
	FVoiceInfo(const FText NewVoiceText, const TSoftObjectPtr<USoundBase> NewVoiceSound) : VoiceText(NewVoiceText),
	VoiceSound(NewVoiceSound) {}

};

USTRUCT(BlueprintType)
struct FPlayVoiceInfo
{
	GENERATED_USTRUCT_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		FText ShowVoiceText = FText();
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		FVoiceInfo VoiceInfo;

	FPlayVoiceInfo() {}
	FPlayVoiceInfo(const FText NewShowVoiceText, const FVoiceInfo NewVoiceInfo) : ShowVoiceText(NewShowVoiceText),
	VoiceInfo(NewVoiceInfo) {}

};

USTRUCT(BlueprintType)
struct FReplyInfo
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		FPlayVoiceInfo ReplyVoiceInfo;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="DialogueInfo")
		int32 ReplyIndex = 0;

	FReplyInfo() {}
	FReplyInfo(const FPlayVoiceInfo NewVoiceInfo, const int32 NewReplyIndex) : ReplyVoiceInfo(NewVoiceInfo), ReplyIndex(NewReplyIndex) {}
	
};

UENUM(BlueprintType)
enum class EDialogueState : uint8
{
	None,
	Speak,
	Reply
};

UENUM(BlueprintType)
enum class ELookDialogueState : uint8
{
	None,
	Socket,
	Location,
	OwnerLocation
};
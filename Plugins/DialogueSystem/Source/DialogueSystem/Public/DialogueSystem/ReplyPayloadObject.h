// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueInfo.h"
#include "ReplyPayloadObject.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnSelectReply, FReplyInfo, ReplyInfo);

UCLASS(BlueprintType)
class DIALOGUESYSTEM_API UReplyPayloadObject : public UObject
{
	GENERATED_BODY()

public:
	
	UFUNCTION(BlueprintCallable)
		FReplyInfo GetReplyInfo() const { return ReplyInfo; }

	UPROPERTY(BlueprintCallable, BlueprintAssignable)
		FOnSelectReply OnSelectReply;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Payload", meta=(AllowPrivateAccess, ExposeOnSpawn))
		FReplyInfo ReplyInfo;
};

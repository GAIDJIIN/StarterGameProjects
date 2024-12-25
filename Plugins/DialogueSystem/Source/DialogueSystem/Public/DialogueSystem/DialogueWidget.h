// Florist Game. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "DialogueComponent.h"
#include "Blueprint/UserWidget.h"
#include "DialogueInfo.h"
#include "DialogueWidget.generated.h"

class UDialogueComponent;

UCLASS(Abstract)
class UDialogueWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	// Main Logic
	void SetupDialogueWidget(UDialogueComponent* NewDialogueComponent);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dialogue Widget")
		void SetupReplies(const TArray<FPlayVoiceInfo>& Replies);
	
	// Getter
	UFUNCTION(BlueprintCallable, Category="Dialogue Widget")
		UDialogueComponent* GetDialogueComponent() const { return DialogueCompRef; }
	
	// Setter
	UFUNCTION(BlueprintCallable, Category="Dialogue Widget")
		void SetDialogueComponent(UDialogueComponent* NewDialogueComponent) { DialogueCompRef = NewDialogueComponent; }
	
protected:

	// Main Logic

	
	// Call after DialogueComp is set
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category="Dialogue Widget")
		void PostSetupDialogueWidget();

	// Getter
	UFUNCTION(BlueprintCallable, Category="Dialogue Widget")
		const FText GetDialogueMemberName(const bool bIsDialogueInstigatorName = false) const
	{
		if(!DialogueCompRef) return FText();
		return DialogueCompRef->GetDialogueMemberName(bIsDialogueInstigatorName);
	}
	
private:
	
	// Variables
	TObjectPtr<UDialogueComponent> DialogueCompRef = nullptr;

};

// Florist Game. All rights reserved.


#include "DialogueSystem/DialogueWidget.h"

void UDialogueWidget::SetupDialogueWidget(UDialogueComponent* NewDialogueComponent)
{
	DialogueCompRef = NewDialogueComponent;
	if(!DialogueCompRef) return;
	PostSetupDialogueWidget();
}

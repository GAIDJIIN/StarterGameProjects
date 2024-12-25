// Florist Game. All rights reserved.


#include "Library/GameEnhancedInputMappingSystemLibrary.h"
#include "Component/GameEnhancedInputComponent.h"

const EPlayerInputMode UGameEnhancedInputMappingSystemLibrary::GetCurrentPlayerInputMode(const APlayerController* PlayerController)
{
	using enum EPlayerInputMode;
	if (PlayerController)
	{
		UGameViewportClient* LocalGameViewportClient = PlayerController->GetWorld()->GetGameViewport();
		ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
		if(!LocalGameViewportClient || !LocalPlayer) return None;
		
		bool LocalbIsIgnore = LocalGameViewportClient->IgnoreInput();
		EMouseCaptureMode LocalCaptureMode = LocalGameViewportClient->GetMouseCaptureMode();

		// UI Only
		if (LocalbIsIgnore && LocalCaptureMode == EMouseCaptureMode::NoCapture) return UI;

		// Game And UI
		if (!LocalbIsIgnore && LocalCaptureMode == EMouseCaptureMode::CaptureDuringMouseDown) return GameAndUI;

		// Game Only
		return Game;
	}

	return None;
}

UGameEnhancedInputComponent* UGameEnhancedInputMappingSystemLibrary::GetGameEnhancedInputComponent(
	const APlayerController* PlayerController)
{
	if(!PlayerController) return nullptr;
	auto LocalGameEnhancedInputComponent = PlayerController->GetComponentByClass<UGameEnhancedInputComponent>();
	return LocalGameEnhancedInputComponent;
}

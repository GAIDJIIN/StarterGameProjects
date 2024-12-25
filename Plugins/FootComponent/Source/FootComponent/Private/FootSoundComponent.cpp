//Florist Game. All rights reserved.


#include "FootSoundComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/StreamableManager.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY(FootSoundComp);

UFootSoundComponent::UFootSoundComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}

void UFootSoundComponent::BeginPlay()
{
	Super::BeginPlay();

	if(!GetOwner()) return;
	OwnerCharRef = Cast<ACharacter>(GetOwner());
	
	SetCheckFootSoundTimer(true);
	
	UE_LOG(LogTemp,Warning,TEXT("Start Check Foot Sound"));
}

void UFootSoundComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

// Setter
void UFootSoundComponent::SetCheckFootSoundTimer(const bool bIsCheck)
{
	if(!GetWorld()) return;
	bIsPlayFootSoundNow = false;
	if(bIsCheck)
	{
		GetWorld()->GetTimerManager().SetTimer(CheckFootSoundTimerHandle,
			this,
			&UFootSoundComponent::CheckSpeedForFootSound,
			CheckSpeedFreq,
			true);
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(CheckFootSoundTimerHandle);
}

void UFootSoundComponent::SetFootSoundTimer()
{
	if(!GetWorld()) return;
	LastSoundPlayTime = GetFootSoundPlayTime();
	GetWorld()->GetTimerManager().SetTimer(PlayFootSoundTimerHandle,
			this,
			&UFootSoundComponent::TryPlayFootSound,
			LastSoundPlayTime);
}

// Getter
const float UFootSoundComponent::GetOwnerXYSpeed() const
{
	if(!GetOwner()) return 0.f;
	return GetOwner()->GetVelocity().Size2D();
}

TSoftObjectPtr<USoundBase> UFootSoundComponent::GetFootSoundbySurface(const TEnumAsByte<EPhysicalSurface> Surface) const
{
	return FootSoundBySurface.FindRef(Surface);
}

const float UFootSoundComponent::GetFootSoundVolumeMultiplier() const
{
	if(!CurveFootSoundVolumeMultiplier) return 0.f;
	return CurveFootSoundVolumeMultiplier->GetFloatValue(GetOwnerXYSpeed());
}

const float UFootSoundComponent::GetFootSoundPlayTime() const
{
	if(!CurveFootSoundPlayTime) return 0.f;
	return CurveFootSoundPlayTime->GetFloatValue(GetOwnerXYSpeed());
}

// Main Logic
void UFootSoundComponent::CheckSpeedForFootSound()
{
	if(!OwnerCharRef.IsValid()) return;
	ShowDebug();
	TrySetFootSoundTimer();
}

void UFootSoundComponent::TryPlayFootSound()
{
	if(!GetWorld()) return;
	if(!OwnerCharRef.IsValid() || !OwnerCharRef->GetCharacterMovement() || !OwnerCharRef->GetCapsuleComponent()) return;
	FFindFloorResult LocalOutFloorResult;
	OwnerCharRef->GetCharacterMovement()->FindFloor(OwnerCharRef->GetCapsuleComponent()->GetComponentLocation(),
		LocalOutFloorResult,
		true);
	if(LocalOutFloorResult.IsWalkableFloor())
	{
		CurrentFootSoundLocation = LocalOutFloorResult.HitResult.ImpactPoint;
		CurrentFootSound = GetFootSoundbySurface(UGameplayStatics::GetSurfaceType(LocalOutFloorResult.HitResult));
		
		if(CurrentFootSound.IsValid()) PlayFootSound();
		else TryLoadAndPlayFootSound();
	}

	SetFootSoundTimer();
}

void UFootSoundComponent::TryLoadAndPlayFootSound()
{
	FStreamableManager Streamable;
	Streamable.RequestAsyncLoad(CurrentFootSound.ToSoftObjectPath(), FStreamableDelegate::CreateUObject(this, &UFootSoundComponent::PlayFootSound));
}

void UFootSoundComponent::PlayFootSound()
{
	if(!GetWorld()) return;
	LastSoundVolumeMultiplier = GetFootSoundVolumeMultiplier();
	
	UGameplayStatics::PlaySoundAtLocation(GetWorld(),
			CurrentFootSound.Get(),
			CurrentFootSoundLocation,
			FRotator::ZeroRotator,
			LastSoundVolumeMultiplier
			);

	if(bShowDebug) UKismetSystemLibrary::PrintString(GetWorld(),
		"FOOT SOUND",
		true,
		true,
		FLinearColor(0.f,0.14326f, 1.f,1.f),
		1.f
		);
}

void UFootSoundComponent::TrySetFootSoundTimer()
{
	if(!GetWorld()) return;
	if(GetOwnerXYSpeed() > 0)
	{
		if(bIsPlayFootSoundNow) return;
		bIsPlayFootSoundNow = true;
		SetFootSoundTimer();
		return;
	}
	GetWorld()->GetTimerManager().ClearTimer(PlayFootSoundTimerHandle);
	bIsPlayFootSoundNow = false;
}

// Debug Logic
void UFootSoundComponent::ShowDebug()
{
	if(!bShowDebug || !GetWorld()) return;
	FString DebugText = "Debug From BPC_FootSound\n";
	DebugText += "\nFoot Sound Ref Is Valid : " + FString(CurrentFootSound.IsValid() ? "True" : "False");
	DebugText += "\nFoot Sound Now : " + FString(bIsPlayFootSoundNow ? "True" : "False");
	DebugText += "\nFoot Sound Timer : " + FString(GetWorld()->GetTimerManager().IsTimerActive(PlayFootSoundTimerHandle) ? "True" : "False");
	DebugText += "\nCheck Foot Sound Timer : " + FString(GetWorld()->GetTimerManager().IsTimerActive(CheckFootSoundTimerHandle) ? "True" : "False");
	DebugText += "\nLast Foot Sound Play Time : " + FString::SanitizeFloat(LastSoundPlayTime);
	DebugText += "\nLast Foot Sound Volume Multiplier : " + FString::SanitizeFloat(LastSoundVolumeMultiplier);

	UKismetSystemLibrary::PrintString(GetWorld(),
			DebugText,
			true,
			true,
			FLinearColor(0.f,0.053782f, 1.f,1.f),
			CheckSpeedFreq
			);
}
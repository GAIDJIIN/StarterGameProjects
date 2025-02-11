// Florist Game. All rights reserved.


#include "Movement/CustomCharacterMovementComp.h"
#include "Movement/MovementInfoInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

void UCustomCharacterMovementComp::TickComponent(float DeltaTime, ELevelTick TickType,
                                                 FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ShowDebug();
}

void UCustomCharacterMovementComp::Deactivate()
{
	Super::Deactivate();

	ResetMovementComp();
}

void UCustomCharacterMovementComp::BeginPlay()
{
	Super::BeginPlay();
	
	if(!GetWorld()) return;

	FString LocalNetMode = "";
	switch (GetNetMode())
	{
	case NM_Standalone: 
		LocalNetMode = "Standalone";
		break;
	case NM_DedicatedServer: 
		LocalNetMode = "DedicatedServer";
		break;
	case NM_ListenServer: 
		LocalNetMode = "ListenServer";
		break;
	case NM_Client: 
		LocalNetMode = "Client";
		break;
	case NM_MAX: 
		LocalNetMode = "MAX";
		break;
	}
	
	if(Cast<APawn>(GetOwner())->IsLocallyControlled()) UE_LOG(LogTemp, Log, TEXT("Net Mode - %s - Start on %s - \n OwnerRole - %s \n RemoteRole - %s"),
		*LocalNetMode, *GetNameSafe(GetOwner()), *UEnum::GetValueAsString(GetOwnerRole()), *UEnum::GetValueAsString(GetOwner()->GetRemoteRole()));
	
	SortCameraShakeBySpeed();
	
		// Set timer for calculate speed by directions
	GetWorld()->GetTimerManager().SetTimer(CalculateMovementInfoTimerHandle, this,
			&UCustomCharacterMovementComp::CalculateMovementInfo, CheckInfoFreq, true);
}

void UCustomCharacterMovementComp::CalculateMovementInfo()
{
	CalculateMaxSpeedByDirection(); // Calculate max speed by direction

	// If server or simulated client on other client local machine
	bool bLocalIsLocal = GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy && GetOwnerRole() == ROLE_Authority;
	//bLocalIsLocal = bLocalIsLocal || 
	
	if(GetOwner()->GetRemoteRole() != ROLE_AutonomousProxy) CalculateCameraShakeBySpeed(); // Calculate camera shake by walk
}

float UCustomCharacterMovementComp::GetMaxSpeed() const
{
	const float DefaultSpeed = Super::GetMaxSpeed();

	switch(MovementMode)
	{
		case MOVE_Walking:
		case MOVE_NavWalking:
			return CurrentMaxSpeed;
		case MOVE_Falling:
		case MOVE_Swimming:
		case MOVE_Flying:
		case MOVE_Custom:
			return DefaultSpeed;
		case MOVE_None:
		default:
			return 0.f;
	}
}

void UCustomCharacterMovementComp::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCustomCharacterMovementComp, CurrentMaxSpeed);
	DOREPLIFETIME(UCustomCharacterMovementComp, bShouldRun);
}


// Reset component info
void UCustomCharacterMovementComp::ResetMovementComp()
{
	bShouldRun = false;
	CurrentMaxSpeed = Super::GetMaxSpeed();
	
	if(GetWorld())
	{
		// Clear calculate speed timer handle
		GetWorld()->GetTimerManager().ClearTimer(CalculateMovementInfoTimerHandle);
		
		if(const auto LocalPlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
		{ StopCurrentCameraShake(LocalPlayerCameraManager); }
		
		CurrentCameraShake = nullptr;
	}
}

void UCustomCharacterMovementComp::SetIsShouldRun_Implementation(const bool NewShouldRun)
{
	bShouldRun = NewShouldRun;
}

bool UCustomCharacterMovementComp::GetIsShouldRun_Implementation() const
{
	if(GetOwner()->Implements<UMovementInfoInterface>())
	{ return IMovementInfoInterface::Execute_GetIsShouldRun(GetOwner()); }
	
	return bShouldRun;
}

float UCustomCharacterMovementComp::GetMaxSpeedByDirection() const
{
	if(!UpdatedComponent) return 0.f;

	const auto LocalCalculatedDirectionAngle = GetDirection();

	#if !UE_BUILD_SHIPPING
		if(bShowDebug)
		{
			GEngine->AddOnScreenDebugMessage(-1, CheckInfoFreq + 0.01f, FColor::Emerald,
				"DotProductAngle - " + FString::SanitizeFloat(LocalCalculatedDirectionAngle));
		}
	#endif
	
	const auto LocalDefaultSpeed = Super::GetMaxSpeed();
	const auto LocalSpeedByDirection = GetIsShouldRun() ? LocalDefaultSpeed : LocalDefaultSpeed / 2;
	const auto LocalFullSpeed = GetIsShouldRun() ? (IsCrouching() ? RunSpeedCrouched : RunSpeed) : LocalDefaultSpeed;
	
	return FMath::GetMappedRangeValueClamped<float>(TRange<float>(0.f, 180.f), TRange<float>(LocalFullSpeed,LocalSpeedByDirection), FMath::Abs(LocalCalculatedDirectionAngle));
}

void UCustomCharacterMovementComp::CalculateMaxSpeedByDirection()
{
	CurrentMaxSpeed = GetMaxSpeedByDirection();
}

float UCustomCharacterMovementComp::GetDirection() const
{
	if (!Velocity.IsNearlyZero())
	{
		const FRotator BaseRotation = GetOwner()->GetActorRotation();
		const FMatrix RotMatrix = FRotationMatrix(BaseRotation);
		const FVector ForwardVector = RotMatrix.GetScaledAxis(EAxis::X);
		const FVector RightVector = RotMatrix.GetScaledAxis(EAxis::Y);
		const FVector NormalizedVel = Velocity.GetSafeNormal2D();

		// get a cos(alpha) of forward vector vs velocity
		const float ForwardCosAngle = static_cast<float>(FVector::DotProduct(ForwardVector, NormalizedVel));
		// now get the alpha and convert to degree
		float ForwardDeltaDegree = FMath::RadiansToDegrees(FMath::Acos(ForwardCosAngle));

		// depending on where right vector is, flip it
		const float RightCosAngle = static_cast<float>(FVector::DotProduct(RightVector, NormalizedVel));
		if (RightCosAngle < 0.f) ForwardDeltaDegree *= -1.f;

		return ForwardDeltaDegree;
	}

	return 0.f;
}


TSubclassOf<UCameraShakeBase> UCustomCharacterMovementComp::GetRecursiveCameraShakeBaseIndex(TArray<float> CameraShakeSpeedArray)
{
	if(CameraShakeSpeedArray.IsEmpty()) return nullptr; // return -1 if array is empty
	if(CameraShakeSpeedArray.Num() == 1) return CameraShakeBySpeed.FindRef(CameraShakeSpeedArray[0]); // return 0 if array length == 1

	// Calculate info for create new camera shake by speed array
	const int32 MidValue = CameraShakeSpeedArray.Num() / 2; // Mid speed index
	const float LocalCurrentSpeed = Velocity.Size2D(); // Current speed
	const float LocalMidValue = CameraShakeSpeedArray[MidValue]; // Middle speed
	TArray<float> LocalNewCameraShakeSpeedArray = {}; // New camera shake speed array
	
	// Create new array for search
	const bool bLocalIsHigherThanMid = LocalCurrentSpeed > LocalMidValue; // Is current speed higher than mid speed
	int32 StartLoopIndex = bLocalIsHigherThanMid ? MidValue : 0;
	int32 StopLoopIndex = bLocalIsHigherThanMid ? CameraShakeSpeedArray.Num() : MidValue;
	for(StartLoopIndex; StartLoopIndex < StopLoopIndex; ++StartLoopIndex)
	{ LocalNewCameraShakeSpeedArray.Add(CameraShakeSpeedArray[StartLoopIndex]); }

	// Recursive find camera shake by speed index
	return GetRecursiveCameraShakeBaseIndex(LocalNewCameraShakeSpeedArray);
}

void UCustomCharacterMovementComp::CalculateCameraShakeBySpeed()
{
	if(CameraShakeBySpeed.IsEmpty()) return;	

	// Get array of speed for search current camera shake class
	TArray<float> LocalCameraShakeSpeedArray = {};
	CameraShakeBySpeed.GetKeys(LocalCameraShakeSpeedArray);
	
	auto LocalCurrentCameraShake = GetRecursiveCameraShakeBaseIndex(LocalCameraShakeSpeedArray);
	if(LocalCurrentCameraShake == nullptr || CurrentCameraShake.GetClass() == LocalCurrentCameraShake) return;
	
	// Play new camera shake
	if(const auto LocalPlayerCameraManager = UGameplayStatics::GetPlayerCameraManager(GetWorld(), 0))
	{
		CurrentCameraShake = StartCameraShake(LocalPlayerCameraManager, LocalCurrentCameraShake);
	}
}

// Camera shake control

UCameraShakeBase* UCustomCharacterMovementComp::StartCameraShake(APlayerCameraManager* PlayerCameraManager, TSubclassOf<UCameraShakeBase> ShakeClass, float Scale,
	ECameraShakePlaySpace PlaySpace, FRotator UserPlaySpaceRot)
{
	if(!PlayerCameraManager) return nullptr;

	StopCurrentCameraShake(PlayerCameraManager); // Stop current camera shake
	return PlayerCameraManager->StartCameraShake(ShakeClass, Scale, PlaySpace, UserPlaySpaceRot);
}

void UCustomCharacterMovementComp::StopCurrentCameraShake(APlayerCameraManager* PlayerCameraManager)
{
	if(!PlayerCameraManager || !CurrentCameraShake) return;

	PlayerCameraManager->StopCameraShake(CurrentCameraShake, false);
}

// Sort camera shake by speed from low to high
void UCustomCharacterMovementComp::SortCameraShakeBySpeed()
{
	CameraShakeBySpeed.KeySort([this](const float& SpeedA, const float& SpeedB)
	{
		return SpeedA < SpeedB;
	});
}

// For editor
#if WITH_EDITOR
void UCustomCharacterMovementComp::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	SortCameraShakeBySpeed();
}
#endif


// Show Debug

#if !UE_BUILD_SHIPPING
void UCustomCharacterMovementComp::ShowDebug()
{
	if(!bShowDebug) return;
	
	FString LocalText = "\nCurrent Max Speed = " + FString::SanitizeFloat(CurrentMaxSpeed);
	LocalText += "\n----------------------------Camera Shake Class----------------------------";
	LocalText += "\nShake Class = " + GetNameSafe(CurrentCameraShake.GetClass());
	LocalText += "\nCurrent Speed = " + FString::SanitizeFloat(Velocity.Size2D());
	LocalText += "\n--------------------------------------------------------------------------";

	GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Emerald, LocalText);
}
#endif
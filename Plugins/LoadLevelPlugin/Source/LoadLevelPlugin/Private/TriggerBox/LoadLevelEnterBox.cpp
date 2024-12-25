// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadLevelEnterBox.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"
#include "LoadLevelComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ALoadLevelEnterBox::ALoadLevelEnterBox()
{
	bIsEnter = false;
	bIsEnabled = false;
	bShowDebug = false;

	LoadLevelComponent = CreateDefaultSubobject<ULoadLevelComponent>("Load Level");
	LoadLevelComponent->OnLoadLevels.AddDynamic(this, &ThisClass::OnLoadLevels);

	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnComponentBeginOverlap);
	GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnComponentEndOverlap);
}

void ALoadLevelEnterBox::RetriggerBox()
{
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FTimerHandle LocalHandle;
	GetWorldTimerManager().SetTimer(LocalHandle, [&]() {GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); }, 0.01f, false);
}

void ALoadLevelEnterBox::SetIsEnabled()
{
	if (bIsEnabled) return;
	RetriggerBox();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	bIsEnabled = true;
}

void ALoadLevelEnterBox::UnloadLevels()
{
	bIsEnter = false;
	if (!IsValid(LoadLevelComponent)) return;
	LoadLevelComponent->UnloadLevels();
	OnDestroyed.RemoveDynamic(this, &ThisClass::OnDestroyed_Event);
}

void ALoadLevelEnterBox::BeginPlay()
{
	Super::BeginPlay();

	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetActorTickEnabled(true);
}

void ALoadLevelEnterBox::Tick(float DeltaSeconds)
{
	if (UBoxComponent* Box = Cast<UBoxComponent>(GetCollisionComponent()))
	{
		UKismetSystemLibrary::DrawDebugBox(GetWorld(), GetCollisionComponent()->GetComponentLocation(), Box->GetScaledBoxExtent(), FLinearColor::Green, GetCollisionComponent()->GetComponentRotation(), 0.f, 10.f);
		if (bFaw) GEngine->AddOnScreenDebugMessage(INDEX_NONE, 0.f, FColor::Orange, *FString::Printf(TEXT("LOADED ALL LEVELS FROM: %s"), *this->GetName()));
	}
}

void ALoadLevelEnterBox::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	TArray<AActor*> OverlappingActors;
	GetCollisionComponent()->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
	if (IsValid(NewPawn) && OverlappingActors.Contains(NewPawn)) RetriggerBox();
}

void ALoadLevelEnterBox::OnDestroyed_Event(AActor* DestroyedActor)
{
	//UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->OnDestroyed.RemoveDynamic(this, &ThisClass::OnDestroyed_Event);

	bIsEnter = false;
	if (IsValid(LoadLevelComponent)) LoadLevelComponent->UnloadLevels();
}

void ALoadLevelEnterBox::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0) && !bIsEnter)
	{
		bIsEnter = true;
		if (IsValid(LoadLevelComponent)) LoadLevelComponent->LoadLevels();
		
		UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->OnDestroyed.AddDynamic(this, &ThisClass::OnDestroyed_Event);
	}
}

void ALoadLevelEnterBox::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor == UGameplayStatics::GetPlayerPawn(GetWorld(), 0))
	{
		bIsEnter = false;
		UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->OnDestroyed.RemoveDynamic(this, &ThisClass::OnDestroyed_Event);
	}
}

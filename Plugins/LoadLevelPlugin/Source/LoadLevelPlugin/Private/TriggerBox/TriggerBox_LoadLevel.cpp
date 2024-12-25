// Fill out your copyright notice in the Description page of Project Settings.


#include "TriggerBox_LoadLevel.h"
#include "LoadLevelComponent.h"
#include "Components/ShapeComponent.h"
#include "Kismet/GameplayStatics.h"

using enum ELoadLevelBoxType;

ATriggerBox_LoadLevel::ATriggerBox_LoadLevel()
{
	LoadLevelComponent = CreateDefaultSubobject<ULoadLevelComponent>("LoadLevel");
	LoadLevelBoxType = ELoadLevelBoxType::LLBT_Both;
	bShowDebug = false;
	bIsEnter = false;
	bFaw = false;
}

void ATriggerBox_LoadLevel::RetriggerBox()
{
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FTimerHandle LocalHandle;
	GetWorldTimerManager().SetTimer(LocalHandle, [&]() {GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); }, 0.1f, false, 0.f);
}

void ATriggerBox_LoadLevel::BeginPlay()
{
	Super::BeginPlay();
	RetriggerBox();

	// Bindings

	UGameplayStatics::GetPlayerController(GetWorld(), 0)->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossesedPawnChanged);
	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnComponentBeginOverlap);
	if (LoadLevelBoxType == LLBT_Both)
		GetCollisionComponent()->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnComponentEndOverlap);
	else if (LoadLevelBoxType == LLBT_Load)
		LoadLevelComponent->OnLoadLevels.AddDynamic(this, &ThisClass::OnLoadLevels);

	SetActorTickEnabled(bShowDebug);
}

void ATriggerBox_LoadLevel::OnPossesedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	TArray<AActor*> OverlappingActors;
	GetCollisionComponent()->GetOverlappingActors(OverlappingActors);
	if (OverlappingActors.Contains(NewPawn)) RetriggerBox();
}

void ATriggerBox_LoadLevel::OnDestroyed(AActor* DestroyedActor)
{

	if (LoadLevelBoxType == LLBT_Both)
	{
		if (!IsValid(DestroyedActor)) return;
		//FBoxSphereBounds LocalBounds = GetCollisionComponent()->Bounds;
		//const FBox Box(LocalBounds.Origin - LocalBounds.BoxExtent, LocalBounds.Origin + LocalBounds.BoxExtent)
		//if (Box.IsInsideOrOn(DestroyedActor->GetActorLocation())) LoadLevelComponent->UnloadLevels();

		TArray<AActor*> LocalOverlappingActors;
		GetCollisionComponent()->GetOverlappingActors(LocalOverlappingActors, APawn::StaticClass());
		if(LocalOverlappingActors.Contains(DestroyedActor)) LoadLevelComponent->UnloadLevels();
	}
	else if (LoadLevelBoxType == LLBT_Load)
	{
		UGameplayStatics::GetPlayerPawn(GetWorld(), 0)->OnDestroyed.RemoveDynamic(this, &ThisClass::OnDestroyed);
		bIsEnter = false;
		LoadLevelComponent->UnloadLevels();
	}
	
}

void ATriggerBox_LoadLevel::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// If not player pawn - discard
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor != PlayerPawn) return;

	if (LoadLevelBoxType != LLBT_Both && !bIsEnter) bIsEnter = true;
	else if (LoadLevelBoxType != LLBT_Both && bIsEnter) return;

	LoadLevelComponent->LoadLevels();
	PlayerPawn->OnDestroyed.AddDynamic(this, &ThisClass::OnDestroyed);
}

void ATriggerBox_LoadLevel::OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	if (OtherActor != PlayerPawn) return;

	LoadLevelComponent->UnloadLevels();
	PlayerPawn->OnDestroyed.RemoveDynamic(this, &ThisClass::OnDestroyed);
}
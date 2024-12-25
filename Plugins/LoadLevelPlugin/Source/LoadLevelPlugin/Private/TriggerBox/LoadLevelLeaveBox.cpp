// Fill out your copyright notice in the Description page of Project Settings.


#include "LoadLevelLeaveBox.h"
#include "LoadLevelEnterBox.h"
#include "Components/ShapeComponent.h"
#include "Components/BoxComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

ALoadLevelLeaveBox::ALoadLevelLeaveBox()
{

	GetCollisionComponent()->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnComponentBeginOverlap);
}

void ALoadLevelLeaveBox::RetriggerBox()
{
	GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	FTimerHandle LocalHandle;
	GetWorldTimerManager().SetTimer(LocalHandle, [&]() {GetCollisionComponent()->SetCollisionEnabled(ECollisionEnabled::QueryOnly); }, 0.01f, false);
}

void ALoadLevelLeaveBox::BeginPlay()
{
	Super::BeginPlay();
	if (EnterBoxesRefs.IsEmpty()) { Destroy(); return; }

	RetriggerBox();
	UGameplayStatics::GetPlayerController(GetWorld(), 0)->OnPossessedPawnChanged.AddDynamic(this, &ThisClass::OnPossessedPawnChanged);
	SetupEnterBoxes();
}

void ALoadLevelLeaveBox::Tick(float DeltaSeconds)
{
	if (UBoxComponent* Box = Cast<UBoxComponent>(GetCollisionComponent()))
		UKismetSystemLibrary::DrawDebugBox(
			GetWorld(),
			GetCollisionComponent()->GetComponentLocation(),
			Box->GetScaledBoxExtent(),
			FLinearColor::Red,
			GetCollisionComponent()->GetComponentRotation(),
			0.f,
			10.f);
}

void ALoadLevelLeaveBox::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	TArray<AActor*> OverlappingActors;
	GetCollisionComponent()->GetOverlappingActors(OverlappingActors, APawn::StaticClass());
	if (IsValid(NewPawn) && OverlappingActors.Contains(NewPawn)) RetriggerBox();
}

void ALoadLevelLeaveBox::OnComponentBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (OtherActor != UGameplayStatics::GetPlayerPawn(GetWorld(), 0)) return;

	for (auto item : EnterBoxesRefs)
		if (IsValid(item)) item->UnloadLevels();
}

void ALoadLevelLeaveBox::SetupEnterBoxes()
{
	for (auto item : EnterBoxesRefs)
	{
		if (!IsValid(item)) continue;
		item->SetIsEnabled();
		if (item->IsShowDebug()) SetActorTickEnabled(true);
	}
}

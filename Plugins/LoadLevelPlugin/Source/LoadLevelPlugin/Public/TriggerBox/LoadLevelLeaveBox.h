// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "LoadLevelLeaveBox.generated.h"

class ALoadLevelEnterBox;

/**
 * 
 */
UCLASS()
class LOADLEVELPLUGIN_API ALoadLevelLeaveBox : public ATriggerBox
{
	GENERATED_BODY()

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Refs") TArray<ALoadLevelEnterBox*> EnterBoxesRefs;
public:
	ALoadLevelLeaveBox();
	void RetriggerBox();

	UFUNCTION(BlueprintCallable) void SetupEnterBoxes();
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UFUNCTION() void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION() void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
};

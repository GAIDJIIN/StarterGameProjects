// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "TriggerBox_LoadLevel.generated.h"

/**
 * 
 */

class ULoadLevelComponent;

UENUM(BlueprintType)
enum class ELoadLevelBoxType : uint8
{
	LLBT_Both = 0	UMETA(DisplayName = "Both"),
	LLBT_Load = 1	UMETA(DisplayName = "Load"),
	LLBT_Unload = 2 UMETA(DisplayName = "Unload"),
};

UCLASS()
class LOADLEVELPLUGIN_API ATriggerBox_LoadLevel : public ATriggerBox
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Actor Components") ULoadLevelComponent* LoadLevelComponent;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup") ELoadLevelBoxType LoadLevelBoxType;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Setup") bool bShowDebug;

	UPROPERTY(EditDefaultsOnly, BLueprintReadWrite, Category = "Service") bool bIsEnter;
	UPROPERTY(EditDefaultsOnly, BLueprintReadWrite, Category = "Service") bool bFaw;
public:
	ATriggerBox_LoadLevel();

	UFUNCTION(BlueprintCallable) void RetriggerBox();
	virtual void BeginPlay() override;

protected:
	UFUNCTION() void OnPossesedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION() void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	UFUNCTION() void OnComponentEndOverlap(
		UPrimitiveComponent* OverlappedComponent, 
		AActor* OtherActor, 
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);
	UFUNCTION() void OnDestroyed(AActor* DestroyedActor);
	UFUNCTION() FORCEINLINE void OnLoadLevels() { bFaw = true; }
};

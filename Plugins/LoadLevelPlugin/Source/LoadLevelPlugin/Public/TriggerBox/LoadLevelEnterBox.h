// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/TriggerBox.h"
#include "LoadLevelEnterBox.generated.h"

class ULoadLevelComponent;

/**
 * 
 */
UCLASS()
class LOADLEVELPLUGIN_API ALoadLevelEnterBox : public ATriggerBox
{
	GENERATED_BODY()
	
public:

protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Actor Components") ULoadLevelComponent* LoadLevelComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug") bool bShowDebug;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bIsEnabled;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Service") bool bIsEnter;
	bool bFaw;
public:
	ALoadLevelEnterBox();
	void RetriggerBox();

	UFUNCTION(BlueprintPure, BlueprintCallable, Category = "Getter") FORCEINLINE bool IsShowDebug() { return bShowDebug; }
	UFUNCTION(BlueprintCallable, Category = "Setter") void SetIsEnabled();
	UFUNCTION(BlueprintCallable, Category = "Trigger Load Level") void UnloadLevels();
protected:

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	UFUNCTION() void OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);
	UFUNCTION() void OnDestroyed_Event(AActor* DestroyedActor);
	UFUNCTION() void OnLoadLevels() { bFaw = true; }
	UFUNCTION() void OnComponentBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp, 
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);
	
	UFUNCTION() void OnComponentEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SoftWorldReference.h"
#include "Engine/LevelStreaming.h"
#include "LevelInstance/LevelInstanceActor.h"
#include "LoadLevelInfo.generated.h"

USTRUCT(BlueprintType)
struct FLoadLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) FSoftWorldReference Level;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LevelTag;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FVector LevelLocation;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FRotator LevelRotation;

};

USTRUCT(BlueprintType)
struct FForToggleLevelInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) TSoftObjectPtr<ALevelInstance> LevelRef;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) FString LevelTag;
};

USTRUCT(BlueprintType)
struct FLoadedLevelStatus
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite) bool bIsLoaded;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<UObject*> PendingToLoadInstigators;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) TArray<UObject*> PendingToUnLoadInstigators;
	UPROPERTY(EditAnywhere, BlueprintReadWrite) ULevelStreaming* LevelReference;
};
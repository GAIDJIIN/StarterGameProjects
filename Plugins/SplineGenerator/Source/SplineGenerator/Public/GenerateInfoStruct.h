// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "GenerateInfoStruct.generated.h"

class UMaterialInterface;
class UStaticMesh;

USTRUCT(BlueprintType)
struct FGenerateRotationInfo
{
	GENERATED_USTRUCT_BODY()
	// Random Rotation
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation")
		FRotator AddRotation = FRotator::ZeroRotator;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation")
		bool SeparateRotationByXYZ = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation",meta=(EditCondition="SeparateRotationByXYZ",EditConditionHides))
		FVector RandomRotationXYZMin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation",meta=(EditCondition="SeparateRotationByXYZ",EditConditionHides))
		FVector RandomRotationXYZMax = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation",meta=(EditCondition="SeparateRotationByXYZ==false",EditConditionHides))
		float RandomRotationMin = 0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation",meta=(EditCondition="SeparateRotationByXYZ==false",EditConditionHides))
		float RandomRotationMax = 0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Rotation")
		float RandomZStepRotation = 0.f;
};

USTRUCT(BlueprintType)
struct FGenerateOffsetInfo
{
	GENERATED_USTRUCT_BODY()
	// Random Offset
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset")
		FVector AddOffset = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset")
		bool SeparateOffsetByXYZ = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset",meta=(EditCondition="SeparateOffsetByXYZ",EditConditionHides))
		FVector RandomOffsetXYZMin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset",meta=(EditCondition="SeparateOffsetByXYZ",EditConditionHides))
		FVector RandomOffsetXYZMax = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset",meta=(EditCondition="SeparateOffsetByXYZ==false",EditConditionHides))
		float RandomOffsetMin = 0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Offset",meta=(EditCondition="SeparateOffsetByXYZ==false",EditConditionHides))
		float RandomOffsetMax = 0.f;
};

USTRUCT(BlueprintType)
struct FGenerateScaleInfo
{
	GENERATED_USTRUCT_BODY()
	// Random Scale
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale")
		FVector AddScale = FVector::One();
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale")
		bool SeparateScaleByXYZ = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale",meta=(EditCondition="SeparateScaleByXYZ",EditConditionHides))
		FVector RandomScaleXYZMin = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale",meta=(EditCondition="SeparateScaleByXYZ",EditConditionHides))
		FVector RandomScaleXYZMax = FVector::ZeroVector;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale",meta=(EditCondition="SeparateScaleByXYZ==false",EditConditionHides))
		float RandomScaleMin = 0.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Scale",meta=(EditCondition="SeparateScaleByXYZ==false",EditConditionHides))
		float RandomScaleMax = 0.f;
};

USTRUCT(BlueprintType)
struct FGenerateInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Distance")
		float SpaceBetweenMeshes = 0.0f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="Distance")
		FRandomStream RandomSeed = 0;
	// Snap Info
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snap")
		bool bSnapToFloor = false;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snap",meta=(EditCondition="bSnapToFloor",EditConditionHides))
		float SnapUpDistance = 250.f;
	UPROPERTY(EditAnywhere,BlueprintReadWrite,Category="Snap",meta=(EditCondition="bSnapToFloor",EditConditionHides))
		float SnapDownDistance = 0.f;
	// Random Rotaion
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		FGenerateRotationInfo GenerateRotationInfo;
	// Random Offset
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		FGenerateOffsetInfo GenerateOffsetInfo;
	// Random Scale
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		FGenerateScaleInfo GenerateScaleInfo;
};

USTRUCT(BlueprintType)
struct FMaterials
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		TArray<TObjectPtr<UMaterialInterface>> Materials = {};
};

USTRUCT(BlueprintType)
struct FGenerateSubMeshInfo
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		TMap<TObjectPtr<UStaticMesh>,FMaterials> SubMeshes = {};
	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category="GenerateInfo")
		FGenerateInfo GenerateInfo;
};

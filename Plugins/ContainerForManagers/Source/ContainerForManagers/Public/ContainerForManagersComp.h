// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ContainerForManagersComp.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CONTAINERFORMANAGERS_API UContainerForManagersComp : public UActorComponent
{
	GENERATED_BODY()

public:	
	UContainerForManagersComp();

	// Setter

	// Set Manager by class
	UFUNCTION(BlueprintCallable, Category = "ContainerForManagers")
		void SetManagerByClass(AActor* NewManager);

	// Delete manager by class
	UFUNCTION(BlueprintCallable, Category = "ContainerForManagers")
		void DeleteManagerByClass(TSubclassOf<AActor> Class);
	
	// Getter

	// Get Manager by class
	UFUNCTION(BlueprintCallable, meta = (DeterminesOutputType = "Class"), Category = "ContainerForManagers")
		AActor* GetManagerByClass(TSubclassOf<AActor> Class);
	UFUNCTION(BlueprintCallable, Category = "ContainerForManagers")
		AActor* GetManagerByClassWithoutCast(TSubclassOf<AActor> Class);
	
private:
	
	// Variables
	
	TArray<AActor*> Managers = {};
		
};

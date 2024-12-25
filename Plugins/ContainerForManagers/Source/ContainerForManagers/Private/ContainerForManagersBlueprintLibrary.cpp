// Fill out your copyright notice in the Description page of Project Settings.


#include "ContainerForManagersBlueprintLibrary.h"

#include "ContainerForManagersComp.h"
#include "GameFramework/GameModeBase.h"


UContainerForManagersComp* UContainerForManagersBlueprintLibrary::GetContainerForManagersFromGM(AGameModeBase* GameModeBase)
{
	if(!GameModeBase) return nullptr;

	return GameModeBase->GetComponentByClass<UContainerForManagersComp>();
}

AActor* UContainerForManagersBlueprintLibrary::GetManagerFromGM(AGameModeBase* GameModeBase, TSubclassOf<AActor> Class)
{
	auto LocalContainerForManagers = GetContainerForManagersFromGM(GameModeBase);
	if(!LocalContainerForManagers) return nullptr;

	return LocalContainerForManagers->GetManagerByClass(Class);
}

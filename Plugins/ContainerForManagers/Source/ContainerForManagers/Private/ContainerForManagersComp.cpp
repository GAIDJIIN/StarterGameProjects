// Fill out your copyright notice in the Description page of Project Settings.


#include "ContainerForManagersComp.h"

#include "Kismet/GameplayStatics.h"

UContainerForManagersComp::UContainerForManagersComp()
{
	PrimaryComponentTick.bCanEverTick = false;
	
}

void UContainerForManagersComp::SetManagerByClass(AActor* NewManager)
{
	int32 LocalManagerIndex = -1;
	if(!Managers.IsEmpty())
	{
		LocalManagerIndex = Managers.IndexOfByPredicate([&](AActor* ManagerElement)
		{
			return ManagerElement->GetClass() == NewManager->GetClass();
		});
	}
	
	// If no manager add new
	if(LocalManagerIndex < 0) Managers.Add(NewManager);
	// Else replace with old manager
	else Managers[LocalManagerIndex] = NewManager;
}

void UContainerForManagersComp::DeleteManagerByClass(TSubclassOf<AActor> Class)
{
	if(!Class || Managers.IsEmpty()) return;

	int32 LocalManagerIndex = -1;
	LocalManagerIndex = LocalManagerIndex = Managers.IndexOfByPredicate([&](AActor* ManagerElement)
		{
			return ManagerElement->GetClass() == Class;
		});

	if(LocalManagerIndex < 0) return;
	
	Managers.RemoveAt(LocalManagerIndex);
}

AActor* UContainerForManagersComp::GetManagerByClass(TSubclassOf<AActor> Class)
{
	return GetManagerByClassWithoutCast(Class);
}

AActor* UContainerForManagersComp::GetManagerByClassWithoutCast(TSubclassOf<AActor> Class)
{
	if(!Class) return nullptr;
	AActor* LocalReturnManager = nullptr;
	if(!Managers.IsEmpty())
	{
		for(const auto& LocalManager : Managers)
		{
			if(LocalManager->GetClass() != Class) continue;
			LocalReturnManager = LocalManager;
			break;
		}
		if(!LocalReturnManager) return nullptr;
	}
	// If no manager in managers array
	else
	{
		LocalReturnManager = UGameplayStatics::GetActorOfClass(GetWorld(), Class);
		if(LocalReturnManager) Managers.Add(LocalReturnManager);
		else return nullptr;
	}
	
	return LocalReturnManager;
}


// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventorySubsystem.h"
#include "ZWInventoryManagerComponent.h"

void UZWInventorySubsystem::RegisterInventory(UZWInventoryManagerComponent* InventoryComponent)
{
	if (InventoryComponent)
	{
		if (const APlayerState* PlayerState = InventoryComponent->GetOwner<APlayerState>())
		{
			PlayerInventories.Add(PlayerState, InventoryComponent);
		}
	}
}

void UZWInventorySubsystem::UnregisterInventory(UZWInventoryManagerComponent* InventoryComponent)
{
	if (InventoryComponent)
	{
		if (const APlayerState* PlayerState = InventoryComponent->GetOwner<APlayerState>())
		{
			PlayerInventories.Remove(PlayerState);
		}
	}
}

UZWInventoryManagerComponent* UZWInventorySubsystem::GetInventoryForPlayer(const APlayerState* PlayerState) const
{
	if (PlayerState)
	{
		const TWeakObjectPtr<UZWInventoryManagerComponent>* FoundInventory = PlayerInventories.Find(PlayerState);
		return FoundInventory ? FoundInventory->Get() : nullptr;
	}
	return nullptr;
}
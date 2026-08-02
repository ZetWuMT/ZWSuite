// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "GameFramework/PlayerState.h"
#include "ZWInventorySubsystem.generated.h"

class UZWInventoryManagerComponent;

/**
 * Subsystem for managing and accessing all inventory components in the game.
 */
UCLASS()
class ZWINVENTORY_API UZWInventorySubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	void RegisterInventory(UZWInventoryManagerComponent* InventoryComponent);
	void UnregisterInventory(UZWInventoryManagerComponent* InventoryComponent);

	UZWInventoryManagerComponent* GetInventoryForPlayer(const APlayerState* PlayerState) const;

private:
	TMap<TWeakObjectPtr<const APlayerState>, TWeakObjectPtr<UZWInventoryManagerComponent>> PlayerInventories;
};
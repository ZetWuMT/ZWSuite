// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ScatteringBase/ZWScatterer.h"
#include "ZWLootScatterer.generated.h"

// ------- Loot Scatter Entry -------------

class UZWInventoryItemDefinition;

USTRUCT(BLueprintType)
struct FZWLootScatterEntry : public FZWScatterEntry
{
	GENERATED_BODY()
	
	// The StaticMesh to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSoftObjectPtr<UStaticMesh> ItemStaticMesh;
	
	// The item to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSoftObjectPtr<UZWInventoryItemDefinition> ItemDefinition;
};

USTRUCT()
struct FZWLootSpawnParams : public FZWEntrySpawnParams
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	FInventoryPickup InventoryPickup;
	
	UPROPERTY(Transient)
	TSoftObjectPtr<UStaticMesh> StaticMesh;
};
// ------- Loot Scatterer ---------

UCLASS()
class ZWSCATTERINGTOOL_API AZWLootScatterer : public AZWScatterer
{
	GENERATED_BODY()

public:
	AZWLootScatterer();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Configuration")
	TArray<FZWLootScatterEntry> ScatterEntryTable;
	
private:
	virtual void PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes) override;
	
	//@TODO: Add option to not spawn at all and to spawn only a globally limited amount of items
};

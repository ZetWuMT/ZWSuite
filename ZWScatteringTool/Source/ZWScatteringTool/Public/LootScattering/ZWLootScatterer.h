// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ZWLootScatterer.generated.h"

// ------- Loot Scatter Entry -------------

class UZWInventoryItemDefinition;

USTRUCT(BLueprintType)
struct FZWLootScatterEntry
{
	GENERATED_BODY()
	
	// The item to spawn
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot")
	TSoftObjectPtr<UZWInventoryItemDefinition> ItemDefinition;
	
	// Minimum amount of items in a single spawned pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Amounts", meta = (ClampMin = "1"))
	int32 MinStackPerProbe = 1;
	
	// Maximum amount of items in a single spawned pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Amounts", meta = (ClampMin = "1"))
	int32 MaxStackPerProbe = 1;
	
	// The maximum number of probes this item can occupy on the level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Limits", meta = (ClampMin = "1"))
	int32 MaxProbesToUse = 1;

	/**
	 * The absolute maximum amount of thie item that can be spawned in total across all probes.
	 * (e.g., Even if we use 10 probes with 5 items each, cap it at 4 total globally)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Limits", meta = (ClampMin = "1"))
	int32 MaxTotalItems = 999;
	
	// If a probe has ANY of these tags, this item will NOT spawn there
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot|Filtering")
	FGameplayTagContainer ExclusionTags;
};


// ------- Loot Scatterer ---------

UCLASS()
class ZWSCATTERINGTOOL_API AZWLootScatterer : public AActor
{
	GENERATED_BODY()

public:
	AZWLootScatterer();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Configuration")
	TArray<FZWLootScatterEntry> LootTable;

protected:
	virtual void BeginPlay() override;
	
private:
	void ScatterLoot();
};

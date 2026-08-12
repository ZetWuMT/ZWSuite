// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ScatteringBase/ZWScatterer.h"
#include "ZWLootScatterer.generated.h"

class UZWInventoryItemDefinition;
class AZWLootPickupActor;

// ------- Loot Scatter Entry -------------

USTRUCT(BlueprintType)
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
struct FZWLootSpawnParams
{
	GENERATED_BODY()
	
	UPROPERTY(Transient)
	FPickupTemplate Template;
	
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

	// The class of pickup to spawn. Defaults to AZWLootPickupActor. Assign a Blueprint
	// child to customize the pickup (e.g. swap the mesh component for a skeletal mesh).
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Configuration")
	TSubclassOf<AZWLootPickupActor> PickupClass;
	
protected:
	virtual int32 GetNumEntries() const override;
	virtual const FZWScatterEntry& GetEntry(int32 Index) const override;
	virtual bool ShouldProcessEntry(int32 Index) const override;
	virtual void PlanAllocation(const FZWScatterEntry& Entry, AZWScatterProbe* Probe, int32 Count) override;
	virtual void SpawnAllocations() override;
	virtual bool GetShuffleEntries() const override { return true; }

private:
	// Probe -> list of (template + mesh) pairs. Each pair spawns its own pickup,
	// so multiple entries on the same probe no longer fight over a single mesh.
	TMap<AZWScatterProbe*, TArray<FZWLootSpawnParams>> PlannedSpawns;
};

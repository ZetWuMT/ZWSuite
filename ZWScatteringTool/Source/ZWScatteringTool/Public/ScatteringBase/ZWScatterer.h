// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZWScatterProbe.h"
#include "GameFramework/Actor.h"
#include "ZWScatterer.generated.h"

USTRUCT(BlueprintType)
struct FZWScatterEntry
{
	GENERATED_BODY()
	
	// Minimum amount of items in a single spawned pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Amounts", meta = (ClampMin = "1"))
	int32 MinStackPerProbe = 1;
	
	// Maximum amount of items in a single spawned pickup
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Amounts", meta = (ClampMin = "1"))
	int32 MaxStackPerProbe = 1;
	
	// The maximum number of probes this item can occupy on the level
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Limits", meta = (ClampMin = "1"))
	int32 MaxProbesToUse = 1;

	/**
	 * The absolute maximum amount of thie item that can be spawned in total across all probes.
	 * (e.g., Even if we use 10 probes with 5 items each, cap it at 4 total globally)
	 */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Limits", meta = (ClampMin = "1"))
	int32 MaxTotalItems = 999;
	
	// If a probe has ANY of these tags, this item will NOT spawn there
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Filtering")
	FGameplayTagContainer ExclusionTags;
	
	// If a probe has ANY of these tags, ONLY THEN this item will spawn there 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Scatter|Filtering")
	FGameplayTagContainer InclusionTags;
};

UCLASS(Abstract)
class ZWSCATTERINGTOOL_API AZWScatterer : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZWScatterer();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Configuration")
	TSubclassOf<AZWScatterProbe> ProbeClass;

protected:
	virtual void BeginPlay() override;
	
	void Scatter();
	
	// HELPER FUNCTION: Universal algorithm for distributing quantities based on base parameters
	TMap<AZWScatterProbe*, int32> CalculateSpawnsForEntry(const FZWScatterEntry& Entry, const TArray<AZWScatterProbe*>& AllProbes);

	// ---- HOOKS FOR DERIVED CLASSES ----
	virtual int32 GetNumEntries() const = 0;
	virtual const FZWScatterEntry& GetEntry(int32 Index) const = 0;
	virtual bool ShouldProcessEntry(int32 Index) const { return true; }
	virtual void PlanAllocation(const FZWScatterEntry& Entry, AZWScatterProbe* Probe, int32 Count) = 0;
	virtual void SpawnAllocations() = 0;
	virtual bool GetConsumeProbesOnAllocation() const { return false; }
	virtual bool GetShuffleEntries() const { return false; }
};

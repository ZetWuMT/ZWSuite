// Fill out your copyright notice in the Description page of Project Settings.


#include "ScatteringBase/ZWScatterer.h"

#include "Algo/RandomShuffle.h"
#include "Kismet/GameplayStatics.h"
#include "ScatteringBase/ZWScatterProbe.h"


// Sets default values
AZWScatterer::AZWScatterer()
{
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AZWScatterer::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		Scatter();
	}
}

void AZWScatterer::Scatter()
{
	if (!ProbeClass) return;

	// 1. Collect the Probes (only the base class)
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, ProbeClass, FoundActors);
	
	TArray<AZWScatterProbe*> AllProbes;
	for (AActor* Actor : FoundActors)
	{
		if (AZWScatterProbe* Probe = Cast<AZWScatterProbe>(Actor))
		{
			AllProbes.Add(Probe);
		}
	}

	if (AllProbes.IsEmpty()) return;

	// 2. PLANNING PHASE (shared by all Scatterers)
	TArray<int32> EntryOrder;
	EntryOrder.Reserve(GetNumEntries());
	for (int32 i = 0; i < GetNumEntries(); ++i)
	{
		EntryOrder.Add(i);
	}

	if (GetShuffleEntries())
	{
		Algo::RandomShuffle(EntryOrder);
	}

	TArray<AZWScatterProbe*> RemainingProbes = AllProbes;

	for (int32 Index : EntryOrder)
	{
		if (!ShouldProcessEntry(Index)) continue;

		const FZWScatterEntry& Entry = GetEntry(Index);
		const TMap<AZWScatterProbe*, int32> EntryAllocations = CalculateSpawnsForEntry(Entry, RemainingProbes);

		for (const TTuple<AZWScatterProbe*, int32>& Allocation : EntryAllocations)
		{
			PlanAllocation(Entry, Allocation.Key, Allocation.Value);

			if (GetConsumeProbesOnAllocation())
			{
				RemainingProbes.Remove(Allocation.Key);
			}
		}
	}

	// 3. SPAWNING PHASE
	SpawnAllocations();

	// 4. CLEANUP PHASE (Map cleanup common to all Scatterers)
	for (AZWScatterProbe* Probe : AllProbes)
	{
		if (IsValid(Probe))
		{
			Probe->Destroy();
		}
	}
}

TMap<AZWScatterProbe*, int32> AZWScatterer::CalculateSpawnsForEntry(const FZWScatterEntry& Entry, const TArray<AZWScatterProbe*>& AllProbes)
{
	TMap<AZWScatterProbe*, int32> ResultSpawns;

	// Filtering
	TArray<AZWScatterProbe*> ValidProbes;
	for (AZWScatterProbe* Probe : AllProbes)
	{
		if (Entry.ExclusionTags.IsValid() && Probe->LocationTags.HasAny(Entry.ExclusionTags)) continue;
		if (Entry.InclusionTags.IsValid() && !Probe->LocationTags.HasAny(Entry.InclusionTags)) continue;
		
		ValidProbes.Add(Probe);
	}

	Algo::RandomShuffle(ValidProbes);

	int32 CurrentTotalSpawned = 0;
	int32 ProbesUsedForThisItem = 0;

	// Calculate the value for each valid Probe
	for (AZWScatterProbe* TargetProbe : ValidProbes)
	{
		if (ProbesUsedForThisItem >= Entry.MaxProbesToUse) break;

		int32 AmountToSpawnHere = FMath::RandRange(Entry.MinStackPerProbe, Entry.MaxStackPerProbe);
		if (CurrentTotalSpawned + AmountToSpawnHere > Entry.MaxTotalItems)
		{
			AmountToSpawnHere = Entry.MaxTotalItems - CurrentTotalSpawned;
		}

		if (AmountToSpawnHere <= 0) break;

		// Save the assignment
		ResultSpawns.Add(TargetProbe, AmountToSpawnHere);

		CurrentTotalSpawned += AmountToSpawnHere;
		ProbesUsedForThisItem++;
	}

	return ResultSpawns;
}

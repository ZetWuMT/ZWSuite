// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyScattering/ZWPawnScatterer.h"

#include "EnemyScattering/ZWPawnProbe.h"


// Sets default values
AZWPawnScatterer::AZWPawnScatterer()
{
	ProbeClass = AZWPawnProbe::StaticClass();
}

void AZWPawnScatterer::PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes)
{
	// Copy of the array of available points, from which we will remove the occupied spots
	TArray<AZWScatterProbe*> RemainingProbes = AvailableProbes;
    
	// Map: on which point which enemy should appear
	TMap<AZWScatterProbe*, TSubclassOf<AActor>> PlannedSpawns;

	// 1. PLANNING PHASE
	for (const FZWPawnScatterEntry& Entry : ScatterEntryTable)
	{
		if (!Entry.EnemyClass) continue;

		// We use the base math function, but pass ONLY the remaining free points
		TMap<AZWScatterProbe*, int32> EntryAllocations = CalculateSpawnsForEntry(Entry, RemainingProbes);

		for (const TTuple<AZWScatterProbe*, int32>& Allocation : EntryAllocations)
		{
			AZWScatterProbe* TargetProbe = Allocation.Key;
            
			// Save the plan
			PlannedSpawns.Add(TargetProbe, Entry.EnemyClass);
            
			// KEY MOMENT: Remove this point from the pool available to the next enemies in the table
			RemainingProbes.Remove(TargetProbe);
		}
	}

	// 2. SPAWNING PHASE
	for (const TTuple<AZWScatterProbe*, TSubclassOf<AActor>>& Spawn : PlannedSpawns)
	{
		AZWScatterProbe* Probe = Spawn.Key;
		TSubclassOf<AActor> ClassToSpawn = Spawn.Value;

		if (ClassToSpawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			SpawnParams.Owner = this;

			GetWorld()->SpawnActor<AActor>(ClassToSpawn, Probe->GetActorTransform(), SpawnParams);
		}
	}
}
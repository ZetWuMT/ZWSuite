// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyScattering/ZWPawnScatterer.h"

#include "EnemyScattering/ZWPawnProbe.h"


// Sets default values
AZWPawnScatterer::AZWPawnScatterer()
{
	ProbeClass = AZWPawnProbe::StaticClass();
}

int32 AZWPawnScatterer::GetNumEntries() const
{
	return ScatterEntryTable.Num();
}

const FZWScatterEntry& AZWPawnScatterer::GetEntry(int32 Index) const
{
	return ScatterEntryTable[Index];
}

bool AZWPawnScatterer::ShouldProcessEntry(int32 Index) const
{
	return ScatterEntryTable[Index].EnemyClass != nullptr;
}

void AZWPawnScatterer::PlanAllocation(const FZWScatterEntry& Entry, AZWScatterProbe* Probe, int32 Count)
{
	const FZWPawnScatterEntry& PawnEntry = static_cast<const FZWPawnScatterEntry&>(Entry);

	// Save the plan
	PlannedSpawns.Add(Probe, PawnEntry.EnemyClass);
}

void AZWPawnScatterer::SpawnAllocations()
{
	for (const TTuple<AZWScatterProbe*, TSubclassOf<AActor>>& Spawn : PlannedSpawns)
	{
		AZWScatterProbe* Probe = Spawn.Key;
		const TSubclassOf<AActor>& ClassToSpawn = Spawn.Value;

		if (ClassToSpawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
			SpawnParams.Owner = this;

			GetWorld()->SpawnActor<AActor>(ClassToSpawn, Probe->GetActorTransform(), SpawnParams);
		}
	}

	PlannedSpawns.Reset();
}

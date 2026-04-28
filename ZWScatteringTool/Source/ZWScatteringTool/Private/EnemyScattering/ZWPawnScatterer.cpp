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
	// Kopia tablicy dostępnych punktów, z której będziemy usuwać zajęte miejsca
	TArray<AZWScatterProbe*> RemainingProbes = AvailableProbes;
    
	// Mapa: na którym punkcie jaki przeciwnik ma się pojawić
	TMap<AZWScatterProbe*, TSubclassOf<AActor>> PlannedSpawns;

	// 1. FAZA PLANOWANIA
	for (const FZWPawnScatterEntry& Entry : ScatterEntryTable)
	{
		if (!Entry.EnemyClass) continue;

		// Używamy bazowej funkcji matematycznej, ale przekazujemy TYLKO pozostałe wolne punkty
		TMap<AZWScatterProbe*, int32> EntryAllocations = CalculateSpawnsForEntry(Entry, RemainingProbes);

		for (const TTuple<AZWScatterProbe*, int32>& Allocation : EntryAllocations)
		{
			AZWScatterProbe* TargetProbe = Allocation.Key;
            
			// Zapisujemy plan
			PlannedSpawns.Add(TargetProbe, Entry.EnemyClass);
            
			// KLUCZOWY MOMENT: Usuwamy ten punkt z puli dostępnych dla następnych przeciwników w tabeli
			RemainingProbes.Remove(TargetProbe);
		}
	}

	// 2. FAZA SPAWNOWANIA
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
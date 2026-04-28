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

	// 1. Zbieramy Proby (tylko bazowej klasy)
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

	// 2. Oddajemy sterowanie do klasy pochodnej (FAZA PLANOWANIA I SPAWNOWANIA)
	PerformScattering(AllProbes);

	// 3. FAZA CZYSZCZENIA (Sprzątanie mapy wspólne dla wszystkich Scattererów)
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

	// Filtrowanie
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

	// Obliczanie wartości dla każdego poprawnego Proba
	for (AZWScatterProbe* TargetProbe : ValidProbes)
	{
		if (ProbesUsedForThisItem >= Entry.MaxProbesToUse) break;

		int32 AmountToSpawnHere = FMath::RandRange(Entry.MinStackPerProbe, Entry.MaxStackPerProbe);
		if (CurrentTotalSpawned + AmountToSpawnHere > Entry.MaxTotalItems)
		{
			AmountToSpawnHere = Entry.MaxTotalItems - CurrentTotalSpawned;
		}

		if (AmountToSpawnHere <= 0) break;

		// Zapisujemy przypisanie
		ResultSpawns.Add(TargetProbe, AmountToSpawnHere);

		CurrentTotalSpawned += AmountToSpawnHere;
		ProbesUsedForThisItem++;
	}

	return ResultSpawns;
}
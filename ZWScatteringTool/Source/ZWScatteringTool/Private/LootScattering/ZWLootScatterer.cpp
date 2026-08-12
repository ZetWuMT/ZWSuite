// Fill out your copyright notice in the Description page of Project Settings.


#include "LootScattering/ZWLootScatterer.h"

#include "Engine/StaticMesh.h"
#include "Kismet/GameplayStatics.h"
#include "LootScattering/ZWLootPickupActor.h"
#include "LootScattering/ZWLootProbe.h"

// Sets default values
AZWLootScatterer::AZWLootScatterer()
{
	ProbeClass = AZWLootProbe::StaticClass();
	PickupClass = AZWLootPickupActor::StaticClass();
}

int32 AZWLootScatterer::GetNumEntries() const
{
	return ScatterEntryTable.Num();
}

const FZWScatterEntry& AZWLootScatterer::GetEntry(int32 Index) const
{
	return ScatterEntryTable[Index];
}

bool AZWLootScatterer::ShouldProcessEntry(int32 Index) const
{
	return !ScatterEntryTable[Index].ItemDefinition.IsNull();
}

void AZWLootScatterer::PlanAllocation(const FZWScatterEntry& Entry, AZWScatterProbe* Probe, int32 Count)
{
	const FZWLootScatterEntry& LootEntry = static_cast<const FZWLootScatterEntry&>(Entry);

	FZWLootSpawnParams Params;
	Params.Template.ItemDef = LootEntry.ItemDefinition;
	Params.Template.StackCount = Count;
	Params.StaticMesh = LootEntry.ItemStaticMesh;

	PlannedSpawns.FindOrAdd(Probe).Add(Params);
}

void AZWLootScatterer::SpawnAllocations()
{
	const TSubclassOf<AZWLootPickupActor> ResolvedPickupClass = PickupClass ? PickupClass : AZWLootPickupActor::StaticClass();

	for (const TTuple<AZWScatterProbe*, TArray<FZWLootSpawnParams>>& PlannedSpawn : PlannedSpawns)
	{
		AZWScatterProbe* Probe = PlannedSpawn.Key;

		const int32 NumPickupsOnProbe = PlannedSpawn.Value.Num();
		int32 SpawnIndex = 0;
		for (const FZWLootSpawnParams& Params : PlannedSpawn.Value)
		{
			FTransform SpawnTransform = Probe->GetActorTransform();
			if (NumPickupsOnProbe > 1)
			{
				const float Angle = (2.f * PI * SpawnIndex) / NumPickupsOnProbe;
				SpawnTransform.AddToTranslation(FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * 120.f);
			}
			++SpawnIndex;

			AZWLootPickupActor* NewPickup = GetWorld()->SpawnActorDeferred<AZWLootPickupActor>(
				ResolvedPickupClass, SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

			if (NewPickup)
			{
				if (UStaticMesh* NewStaticMesh = Params.StaticMesh.LoadSynchronous())
				{
					NewPickup->SetStaticMesh(NewStaticMesh);
				}

				FInventoryPickup PickupInventory;
				PickupInventory.Templates.Add(Params.Template);
				NewPickup->SetPickupInventory(PickupInventory);

				UGameplayStatics::FinishSpawningActor(NewPickup, SpawnTransform);
			}
		}
	}

	PlannedSpawns.Reset();
}

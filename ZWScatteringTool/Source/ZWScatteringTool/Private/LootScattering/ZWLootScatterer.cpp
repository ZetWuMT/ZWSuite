// Fill out your copyright notice in the Description page of Project Settings.


#include "LootScattering/ZWLootScatterer.h"
#include "IPickupable.h"
#include "ZWInventoryComponent.h"
#include "../../../../../ZWInteraction/Source/ZWInteraction/Public/ZWInteractionComponent.h"
#include "Algo/RandomShuffle.h"
#include "Engine/StaticMeshActor.h"
#include "Kismet/GameplayStatics.h"
#include "LootScattering/ZWLootProbe.h"

// Sets default values
AZWLootScatterer::AZWLootScatterer()
{
	ProbeClass = AZWLootProbe::StaticClass();
}

void AZWLootScatterer::PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes)
{
	TMap<AZWScatterProbe*, FZWLootSpawnParams> PlannedSpawns;
	
	Algo::RandomShuffle(ScatterEntryTable);

	// 1. PLANNING PHASE
	for (const FZWLootScatterEntry& Entry : ScatterEntryTable)
	{
		if (Entry.ItemDefinition.IsNull()) continue;

		// We use the base class method that returns a ready placement plan for this entry!
		TMap<AZWScatterProbe*, int32> EntryAllocations = CalculateSpawnsForEntry(Entry, AvailableProbes);

		for (const TTuple<AZWScatterProbe*, int32>& Allocation : EntryAllocations)
		{
			AZWScatterProbe* TargetProbe = Allocation.Key;
			int32 AmountToSpawn = Allocation.Value;

			FPickupTemplate NewTemplate;
			NewTemplate.ItemDef = Entry.ItemDefinition;
			NewTemplate.StackCount = AmountToSpawn;

			// Add to the Probe in our own dictionary (Map)
			PlannedSpawns.FindOrAdd(TargetProbe).InventoryPickup.Templates.Add(NewTemplate);
			PlannedSpawns.Find(TargetProbe)->StaticMesh = Entry.ItemStaticMesh;
		}
	}

	// 2. SPAWNING PHASE (Remains 100% from your old logic)
	for (const TTuple<AZWScatterProbe*, FZWLootSpawnParams>& PlannedSpawn : PlannedSpawns)
	{
		AZWScatterProbe* Probe = PlannedSpawn.Key;
		const FInventoryPickup& PickupDataToGrant = PlannedSpawn.Value.InventoryPickup;

		FTransform SpawnTransform = Probe->GetActorTransform();
		
		AActor* NewPickup = GetWorld()->SpawnActorDeferred<AActor>(
			AStaticMeshActor::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (NewPickup)
		{
			if (UStaticMeshComponent* StaticMeshComponent = NewPickup->FindComponentByClass<UStaticMeshComponent>())
			{
				if (UStaticMesh* NewStaticMesh = PlannedSpawn.Value.StaticMesh.LoadSynchronous())
				{
					StaticMeshComponent->SetStaticMesh(NewStaticMesh);	
				}				
			}
			
			UActorComponent* NewIntComp = NewPickup->AddComponentByClass(UZWInteractionComponent::StaticClass(), false, FTransform::Identity, true);
			if (NewIntComp) NewIntComp->RegisterComponent();

			UActorComponent* NewInvComp = NewPickup->AddComponentByClass(UZWInventoryComponent::StaticClass(), false, FTransform::Identity, false);
			if (NewInvComp) NewInvComp->RegisterComponent();

			if (UZWInventoryComponent* InvComp = Cast<UZWInventoryComponent>(NewInvComp))
			{
				InvComp->SetPickupInventory(PickupDataToGrant);
			}

			UGameplayStatics::FinishSpawningActor(NewPickup, SpawnTransform);
		}
	}
}
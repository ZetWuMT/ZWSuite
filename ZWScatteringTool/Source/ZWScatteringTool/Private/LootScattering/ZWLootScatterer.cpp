// Fill out your copyright notice in the Description page of Project Settings.


#include "LootScattering/ZWLootScatterer.h"
#include "IPickupable.h"
#include "ZWInventoryComponent.h"
#include "../../../../../ZWInteraction/Source/ZWInteraction/Public/ZWInteractionComponent.h"
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

	// 1. FAZA PLANOWANIA
	for (const FZWLootScatterEntry& Entry : ScatterEntryTable)
	{
		if (Entry.ItemDefinition.IsNull()) continue;

		// Używamy metody z klasy bazowej, która oddaje nam gotowy plan rozlokowania dla tego wpisu!
		TMap<AZWScatterProbe*, int32> EntryAllocations = CalculateSpawnsForEntry(Entry, AvailableProbes);

		for (const TTuple<AZWScatterProbe*, int32>& Allocation : EntryAllocations)
		{
			AZWScatterProbe* TargetProbe = Allocation.Key;
			int32 AmountToSpawn = Allocation.Value;

			FPickupTemplate NewTemplate;
			NewTemplate.ItemDef = Entry.ItemDefinition;
			NewTemplate.StackCount = AmountToSpawn;

			// Dodajemy do Proba w naszym własnym słowniku (Mapie)
			PlannedSpawns.FindOrAdd(TargetProbe).InventoryPickup.Templates.Add(NewTemplate);
			PlannedSpawns.Find(TargetProbe)->StaticMesh = Entry.ItemStaticMesh;
		}
	}

	// 2. FAZA SPAWNOWANIA (Pozostaje w 100% z Twojej starej logiki)
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
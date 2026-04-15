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
	PrimaryActorTick.bCanEverTick = false;
}

// Called when the game starts or when spawned
void AZWLootScatterer::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		ScatterLoot();
	}
}

void AZWLootScatterer::ScatterLoot()
{
	// 1. Zbieramy wszystkie Proby z levelu
	TArray<AActor*> FoundActors;
	UGameplayStatics::GetAllActorsOfClass(this, AZWLootProbe::StaticClass(), FoundActors);

	TArray<AZWLootProbe*> AllProbes;
	for (AActor* Actor : FoundActors)
	{
		if (AZWLootProbe* Probe = Cast<AZWLootProbe>(Actor))
		{
			AllProbes.Add(Probe);
		}
	}

	if (AllProbes.IsEmpty()) return;

	// NOWOŚĆ: Mapa przechowująca zaplanowany loot dla danego punktu (Proba)
	TMap<AZWLootProbe*, FZWLootSpawnParams> PlannedSpawns;

	// 2. FAZA PLANOWANIA (Logika z poprzedniego kroku, ale bez spawnowania)
	for (const FZWLootScatterEntry& Entry : LootTable)
	{
		if (Entry.ItemDefinition.IsNull()) continue;

		TArray<AZWLootProbe*> ValidProbes;
		for (AZWLootProbe* Probe : AllProbes)
		{
			// Opcjonalnie: Jeśli nie chcesz, żeby przedmioty się łączyły, odkomentuj poniższe.
			// Ale zazwyczaj chcemy łączyć loot w "skrzynkach", więc to omijamy!
			// if (PlannedSpawns.Contains(Probe)) continue; 

			if (Entry.ExclusionTags.IsValid() && Probe->LocationTags.HasAny(Entry.ExclusionTags))
			{
				continue;
			}
			ValidProbes.Add(Probe);
		}

		Algo::RandomShuffle(ValidProbes);

		int32 CurrentTotalSpawned = 0;
		int32 ProbesUsedForThisItem = 0;

		for (AZWLootProbe* TargetProbe : ValidProbes)
		{
			if (ProbesUsedForThisItem >= Entry.MaxProbesToUse) break;

			int32 AmountToSpawnHere = FMath::RandRange(Entry.MinStackPerProbe, Entry.MaxStackPerProbe);
			if (CurrentTotalSpawned + AmountToSpawnHere > Entry.MaxTotalItems)
			{
				AmountToSpawnHere = Entry.MaxTotalItems - CurrentTotalSpawned;
			}

			if (AmountToSpawnHere <= 0) break;

			// --- ZAPISUJEMY W MAPIE (ZAMIAST SPAWNOWAĆ) ---
			
			// Tworzymy Template z Twojej struktury
			FPickupTemplate NewTemplate;
			NewTemplate.ItemDef = Entry.ItemDefinition;
			NewTemplate.StackCount = AmountToSpawnHere;

			// Dodajemy do Proba (jeśli Prob nie miał jeszcze lootu, TMap stworzy nowy wpis automatycznie)
			PlannedSpawns.FindOrAdd(TargetProbe).InventoryPickup.Templates.Add(NewTemplate);
			PlannedSpawns.Find(TargetProbe)->StaticMesh = Entry.ItemStaticMesh;

			// Aktualizacja liczników
			CurrentTotalSpawned += AmountToSpawnHere;
			ProbesUsedForThisItem++;
		}
	}

	// 3. FAZA SPAWNOWANIA (Fizyczne pojawienie się Aktorów w świecie)
	for (const TTuple<AZWLootProbe*, FZWLootSpawnParams>& PlannedSpawn : PlannedSpawns)
	{
		AZWLootProbe* Probe = PlannedSpawn.Key;
		const FInventoryPickup& PickupDataToGrant = PlannedSpawn.Value.InventoryPickup;

		FTransform SpawnTransform = Probe->GetActorTransform();
		
		AActor* NewPickup = GetWorld()->SpawnActorDeferred<AActor>(
			AStaticMeshActor::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

		if (NewPickup)
		{
			// UWAGA: Tutaj przypisujesz dane do swojego komponentu/aktora.
			// Zakładam, że Twój Aktor Pickupa ma InventoryComponent, a ten ma zmienną typu FInventoryPickup
			
			if (UStaticMeshComponent* StaticMeshComponent = NewPickup->FindComponentByClass<UStaticMeshComponent>())
			{
				if (UStaticMesh* NewStaticMesh = PlannedSpawn.Value.StaticMesh.LoadSynchronous())
				{
					StaticMeshComponent->SetStaticMesh(NewStaticMesh);;	
				}				
			}
			
			UActorComponent* NewIntComp = NewPickup->AddComponentByClass(UZWInteractionComponent::StaticClass(), false, FTransform::Identity, true);
			if (!NewIntComp)
			{
				UE_LOG(LogTemp, Error, TEXT("Loot Scatterer: New Interaction Component not created!"));
				NewPickup->Destroy(); // Usuń niekompletny aktor
				continue; // Przejdź do następnego zaplanowanego spawnu
			}
			NewIntComp->RegisterComponent();

			UActorComponent* NewInvComp = NewPickup->AddComponentByClass(UZWInventoryComponent::StaticClass(), false, FTransform::Identity, false);
			if (!NewInvComp)
			{
				UE_LOG(LogTemp, Error, TEXT("Loot Scatterer: New Inventory Component not created!"));
				NewPickup->Destroy(); // Usuń niekompletny aktor
				continue; // Przejdź do następnego zaplanowanego spawnu
			}
			NewInvComp->RegisterComponent();

			if (UZWInventoryComponent* InvComp = Cast<UZWInventoryComponent>(NewInvComp))
			{
				
				InvComp->SetPickupInventory(PickupDataToGrant);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("Loot Scatterer: Failed to cast NewInvComp to UZWInventoryComponent for actor %s."), *NewPickup->GetName());
			}

			// W tym miejscu wstrzykujesz zebraną paczkę z mapy do nowo powstałego Aktora.
			// NewPickup->TwojaZmiennaFInventoryPickup = PickupDataToGrant;

			UGameplayStatics::FinishSpawningActor(NewPickup, SpawnTransform);
		}
	}	
	
	// 4. FAZA CZYSZCZENIA (Sprzątanie mapy)
	// Iterujemy po oryginalnej tablicy wszystkich pobranych punktów i niszczymy je.
	for (AZWLootProbe* Probe : AllProbes)
	{
		if (IsValid(Probe))
		{
			Probe->Destroy();
		}
	}
}
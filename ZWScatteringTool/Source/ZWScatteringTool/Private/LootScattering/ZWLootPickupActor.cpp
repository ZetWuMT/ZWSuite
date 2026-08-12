// Fill out your copyright notice in the Description page of Project Settings.


#include "LootScattering/ZWLootPickupActor.h"

#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "ZWInteractionComponent.h"
#include "ZWInventoryComponent.h"

AZWLootPickupActor::AZWLootPickupActor()
{
	PrimaryActorTick.bCanEverTick = false;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	RootComponent = SceneRoot;

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(SceneRoot);

	InteractionComp = CreateDefaultSubobject<UZWInteractionComponent>(TEXT("InteractionComp"));

	InventoryComp = CreateDefaultSubobject<UZWInventoryComponent>(TEXT("InventoryComp"));
}

void AZWLootPickupActor::SetStaticMesh(UStaticMesh* InMesh)
{
	if (InMesh && StaticMeshComp)
	{
		StaticMeshComp->SetStaticMesh(InMesh);
	}
}

void AZWLootPickupActor::SetPickupInventory(const FInventoryPickup& InPickupInventory)
{
	if (InventoryComp)
	{
		InventoryComp->SetPickupInventory(InPickupInventory);
	}
}

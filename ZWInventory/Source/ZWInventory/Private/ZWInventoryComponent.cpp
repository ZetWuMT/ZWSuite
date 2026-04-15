// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryComponent.h"

#include "ZWInteractionComponent.h"
#include "ZWInventoryManagerComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerState.h"
#include "Kismet/GameplayStatics.h"

// Sets default values for this component's properties
UZWInventoryComponent::UZWInventoryComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	if (!InteractionComponent)
	{
		InteractionComponent = CreateDefaultSubobject<UZWInteractionComponent>(TEXT("InteractionComponent"));
	}
}

FInventoryPickup UZWInventoryComponent::GetPickupInventory() const
{
	return StaticInventory;
}

void UZWInventoryComponent::SetPickupInventory(const FInventoryPickup& InPickupInventory)
{
	StaticInventory = InPickupInventory;
}

void UZWInventoryComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InteractionComponent = GetOwner()->FindComponentByClass<UZWInteractionComponent>();
	
	if (IsValid(InteractionComponent) && !InteractionComponent->OnInteract.IsAlreadyBound(this, &UZWInventoryComponent::SetupInteraction))
	{
		InteractionComponent->OnInteract.AddDynamic(this, &UZWInventoryComponent::SetupInteraction);	
	}	
}

void UZWInventoryComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	if (IsValid(InteractionComponent) && InteractionComponent->OnInteract.IsAlreadyBound(this, &UZWInventoryComponent::SetupInteraction))
	{
		InteractionComponent->OnInteract.RemoveDynamic(this, &UZWInventoryComponent::SetupInteraction);	
	}
	
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UZWInventoryComponent::SetupInteraction()
{
	//UInventoryManagerComponent* ManagerComponent = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->FindComponentByClass<UInventoryManagerComponent>();
	UZWInventoryManagerComponent* ManagerComponent = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0)->GetPlayerState()->FindComponentByClass<UZWInventoryManagerComponent>();
	if (ManagerComponent != nullptr && InteractionComponent != nullptr)
	{
		UPickupableStatics::AddPickupToInventory(ManagerComponent, this);
		//InteractionComponent->NotifyFlowGraph();
		GetOwner()->Destroy();
	}
}

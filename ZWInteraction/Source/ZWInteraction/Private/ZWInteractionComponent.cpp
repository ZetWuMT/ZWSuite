// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionComponent.h"

#include "ZWInteractionSystemSettings.h"
#include "Modules/ModuleManager.h"

UZWInteractionComponent::UZWInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UZWInteractionComponent::Interact()
{
	OnInteract.Broadcast();
}

void UZWInteractionComponent::ToggleHighlight(bool IsHighlighted)
{
	bool bShouldBeHighlighted = false;
	if (StaticMeshComponent != nullptr)
	{
		StaticMeshComponent->SetRenderCustomDepth(IsHighlighted);
		bShouldBeHighlighted = IsHighlighted;
	}
	if (SkeletalMeshComponent != nullptr)
	{
		SkeletalMeshComponent->SetRenderCustomDepth(IsHighlighted);
		bShouldBeHighlighted = IsHighlighted;
	}
	
	bIsHighlighted = bShouldBeHighlighted;
}

void UZWInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	
	// Setting up Interaction Response for a Static Mesh Component 
	UActorComponent* ActorComponent = Owner->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (ActorComponent == nullptr)
	{
		ActorComponent = Owner->FindComponentByTag(UStaticMeshComponent::StaticClass(), FName(TEXT("MainInteractionMesh")));
	}
	StaticMeshComponent = Cast<UStaticMeshComponent>(ActorComponent);	
	if (StaticMeshComponent != nullptr)
	{
		StaticMeshComponent->SetCustomDepthStencilValue(1);

		if (const UZWInteractionSystemSettings* Settings = GetDefault<UZWInteractionSystemSettings>())
		{
			StaticMeshComponent->SetCollisionResponseToChannel(Settings->InteractionCollisionChannel, ECollisionResponse::ECR_Block);
		}
	}	
	
	// Setting up Interaction Response for a Skeletal Mesh Component
	ActorComponent = Owner->GetComponentByClass(USkeletalMeshComponent::StaticClass());
	if (ActorComponent == nullptr)
	{
		ActorComponent = Owner->FindComponentByTag(USkeletalMeshComponent::StaticClass(), FName(TEXT("MainInteractionMesh")));
	}
	SkeletalMeshComponent = Cast<USkeletalMeshComponent>(ActorComponent);	
	if (SkeletalMeshComponent != nullptr)
	{
		SkeletalMeshComponent->SetCustomDepthStencilValue(1);

		if (const UZWInteractionSystemSettings* Settings = GetDefault<UZWInteractionSystemSettings>())
		{
			SkeletalMeshComponent->SetCollisionResponseToChannel(Settings->InteractionCollisionChannel, ECollisionResponse::ECR_Block);
		}
	}	
}
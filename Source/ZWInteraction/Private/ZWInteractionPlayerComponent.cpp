// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionPlayerComponent.h"
#include "ZWInteractionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UZWInteractionComponent* UZWInteractionPlayerComponent::GetInteractableObjectInteractionComponent(AActor* InteractableObject)
{
	if (!IsValid(InteractableObject)) 
	{ 
		return nullptr; 
	}
	
	return InteractableObject->GetComponentByClass<UZWInteractionComponent>();
}

void UZWInteractionPlayerComponent::SetInteractableObject(TObjectPtr<UZWInteractionComponent> Object)
{
	if (IsValid(Object))
	{
		InteractableObject = Object;
	}
}

void UZWInteractionPlayerComponent::ResetInteractableObject()
{
	if (InteractableObject)
	{
		InteractableObject->ToggleHighlight(false);
		InteractableObject = nullptr;
	}
}

void UZWInteractionPlayerComponent::SetInteractedObject(TObjectPtr<UZWInteractionComponent> Object)
{
	InteractedObject = Object;
}

void UZWInteractionPlayerComponent::ResetInteractedObject()
{
	InteractedObject = nullptr;
}

// Sets default values for this component's properties
UZWInteractionPlayerComponent::UZWInteractionPlayerComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

// Called when the game starts
void UZWInteractionPlayerComponent::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void UZWInteractionPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DetectInteractiveObjects();
}

void UZWInteractionPlayerComponent::DetectInteractiveObjects()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	FVector DetectorLocation = /*InteractionSubsystem->IsPlayerInvestigating() ? InteractionSubsystem->GetCameraLocation() : */GetComponentLocation();
	FRotator DetectorRotation = /*InteractionSubsystem->IsPlayerInvestigating() ? InteractionSubsystem->GetCameraRotation() : */GetComponentRotation();

	FVector Start = DetectorLocation;
	FVector End = Start + UKismetMathLibrary::GetForwardVector(DetectorRotation) * 300;
	float TraceRadius = /*InteractionSubsystem->IsPlayerInvestigating() ? 5.f : */7.f;

	
	FHitResult HitResult;
	bool Hit;

	Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), Start, End, TraceRadius,
	UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false, ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
	
	if (Hit)
	{
		ResolveInteractiveObject(HitResult.GetActor());
	}
	else if (GetInteractableObject() != nullptr)
	{
		ResetInteractableObject();
	}
}

void UZWInteractionPlayerComponent::ResolveInteractiveObject(AActor* NewInteractableObject)
{
	ResetInteractableObject();

	if (IsValid(GetInteractedObject()) && GetInteractedObject()->GetOwner() == NewInteractableObject) { return; }
		
	UZWInteractionComponent* InteractableObject = GetInteractableObjectInteractionComponent(NewInteractableObject);

	if (InteractableObject)
	{
		{
			SetInteractableObject(InteractableObject);
			InteractableObject->ToggleHighlight(true);
		}				
	}
	else
	{
		ResetInteractableObject();
	}
}

void UZWInteractionPlayerComponent::Interact()
{
	//bool IsInvestigating = InteractionSubsystem->IsPlayerInvestigating();

	TObjectPtr<UZWInteractionComponent> InteractableObject = GetInteractableObject();
	if (InteractableObject != nullptr && InteractableObject->IsHighlighted())
	{
		/*if (InteractableObject->IsInspectable())
		{
			InteractableObject->Inspect();
			return;
		}

		if (InteractableObject->IsInvestigatable())
		{
			InteractableObject->Investigate();
			return;
		}*/

		InteractableObject->Interact();			
	}
	/*else
	{
		if (IsInvestigating)
		{
			InteractionSubsystem->ResolveInvestigation(nullptr);
		}
	}*/
}
/*
void UZWInteractionPlayerComponent::EndInvestigation()
{
	if (InteractionSubsystem->IsPlayerInvestigating())
	{
		InteractionSubsystem->EndInvestigation();
	}
}

void UZWInteractionPlayerComponent::InvestigationUpperLayer()
{
	if (InteractionSubsystem->IsPlayerInvestigating())
	{
		InteractionSubsystem->ResolveInvestigation(nullptr);
	}
}

bool UZWInteractionPlayerComponent::IsInspecting()
{
	return InteractionSubsystem->IsPlayerInspecting();
}

bool UZWInteractionPlayerComponent::IsInvestigating()
{
	return InteractionSubsystem->IsPlayerInvestigating();
}*/
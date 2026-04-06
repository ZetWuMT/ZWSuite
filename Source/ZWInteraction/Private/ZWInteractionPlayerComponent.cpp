// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionPlayerComponent.h"
#include "ZWInteractionComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UZWInteractionPlayerComponent::UZWInteractionPlayerComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UZWInteractionPlayerComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	DetectInteractiveObjects();
}

void UZWInteractionPlayerComponent::Interact()
{
	TObjectPtr<UZWInteractionComponent> InteractionComponent = GetInteractableObject();
	if (InteractionComponent != nullptr && InteractionComponent->IsHighlighted())
	{
		InteractionComponent->Interact();			
	}
}

void UZWInteractionPlayerComponent::ResolveLineTracePoints(FVector& TraceStart, FVector& TraceEnd, float& TraceRadius)
{
	const FVector DetectorLocation = GetComponentLocation();
	const FRotator DetectorRotation = GetComponentRotation();
	
	TraceStart = DetectorLocation;
	TraceEnd = TraceStart + UKismetMathLibrary::GetForwardVector(DetectorRotation) * 300;

	TraceRadius = 7.f;
}

UZWInteractionComponent* UZWInteractionPlayerComponent::GetInteractableObjectInteractionComponent(AActor* InteractableActor)
{
	if (!IsValid(InteractableActor)) 
	{ 
		return nullptr; 
	}
	
	return InteractableActor->GetComponentByClass<UZWInteractionComponent>();
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

void UZWInteractionPlayerComponent::DetectInteractiveObjects()
{
	TArray<AActor*> ActorsToIgnore;
	ActorsToIgnore.Add(GetOwner());

	FVector TraceStart;
	FVector TraceEnd;
	float TraceRadius;
	
	ResolveLineTracePoints(TraceStart, TraceEnd, TraceRadius);
	
	FHitResult HitResult;

	bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, TraceRadius,
													   UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel2), false,
													   ActorsToIgnore, EDrawDebugTrace::None, HitResult, true);
	
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

	if (UZWInteractionComponent* InteractionComponent = GetInteractableObjectInteractionComponent(NewInteractableObject))
	{
		{
			SetInteractableObject(InteractionComponent);
			InteractionComponent->ToggleHighlight(true);
		}				
	}
	else
	{
		ResetInteractableObject();
	}
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionPlayerComponent.h"

#include "ScreenPass.h"
#include "ZWInteractionComponent.h"
#include "ZWInteractionSystemSettings.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
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
	FVector DetectorLocation = GetComponentLocation();
	FVector DetectorDirection = UKismetMathLibrary::GetForwardVector(GetComponentRotation());
	if (APlayerController* PC = Cast<APlayerController>(GetOwner()->GetInstigatorController()))
	{
		int32 ScreenSizeX, ScreenSizeY;
		PC->GetViewportSize(ScreenSizeX, ScreenSizeY);
		PC->DeprojectScreenPositionToWorld(ScreenSizeX * 0.5f, ScreenSizeY * 0.5f, DetectorLocation, DetectorDirection);
	}
	
	TraceStart = DetectorLocation;
	TraceEnd = TraceStart + DetectorDirection * DetectionRange;

	TraceRadius = DetectionTraceRadius;	
}

void UZWInteractionPlayerComponent::ResolveLineTraceIgnoredActors(TArray<AActor*>& ActorsToIgnore)
{
	//Do nothing
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
	
	ResolveLineTracePoints(TraceStart, TraceEnd, DetectionTraceRadius);
	ResolveLineTraceIgnoredActors(ActorsToIgnore);
	
	FHitResult HitResult;

	const UZWInteractionSystemSettings* Settings = GetDefault<UZWInteractionSystemSettings>();
	
	if (!Settings) return;
	
	ETraceTypeQuery CollisionTrace = UEngineTypes::ConvertToTraceType(Settings->InteractionCollisionChannel);
	
	EDrawDebugTrace::Type DetectionDrawDebugTrace = bDrawDebugTrace ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None;

	bool Hit = UKismetSystemLibrary::SphereTraceSingle(GetWorld(), TraceStart, TraceEnd, DetectionTraceRadius,
													   CollisionTrace, false,
													   ActorsToIgnore, DetectionDrawDebugTrace, HitResult, true);
	
	if (Hit)
	{
		ResolveInteractiveObject(HitResult.GetActor());
		GetInteractableObjectName(HitResult.GetActor()->GetName());
		if (bPrintInteractableObjectName && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(1337, 0.0f, FColor::Green, FString::Printf(TEXT("Interactable: %s"), *HitResult.GetActor()->GetName()));
		}
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
		if (InteractionComponent->IsActive())
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
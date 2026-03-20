// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionSubsystem.h"
#include "ZWInteractionComponent.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"

void UZWInteractionSubsystem::SetInteractableObject(TObjectPtr<UZWInteractionComponent> Object)
{
	if (IsValid(Object))
	{
		InteractableObject = Object;
	}
}

void UZWInteractionSubsystem::ResetInteractableObject()
{
	if (InteractableObject)
	{
		InteractableObject->ToggleHighlight(false);
		InteractableObject = nullptr;
	}
}

void UZWInteractionSubsystem::SetInteractedObject(TObjectPtr<UZWInteractionComponent> Object)
{
	InteractedObject = Object;
}

void UZWInteractionSubsystem::ResetInteractedObject()
{
	InteractedObject = nullptr;
}
/*
void UZWInteractionSubsystem::SpawnInteractionSceneCapture()
{
	if (!IsValid(InteractionSceneCapture))
	{
		FVector Location(0, 0, -2000);
		FRotator Rotation(0, 0, 0);
		FActorSpawnParameters SpawnInfo;
		InteractionSceneCapture = GetWorld()->SpawnActor<AInteractionSceneCapture>(Location, Rotation, SpawnInfo);
	}	
}

void UZWInteractionSubsystem::DestroyInteractionSceneCapture()
{
	InteractionSceneCapture->Destroy();
	InteractionSceneCapture = nullptr;
}

void UZWInteractionSubsystem::SetCamera(AActor* NewTarget)
{
	APlayerController* Controller = GetWorld()->GetFirstPlayerController();
	if (!IsValid(Controller) || !IsValid((NewTarget)))
	{
		return;
	}
	
	Controller->SetViewTargetWithBlend(NewTarget, 1, EViewTargetBlendFunction::VTBlend_Linear);
	ActiveCamera = Cast<UCameraComponent>(NewTarget->FindComponentByClass(UCameraComponent::StaticClass()));
	
	if (IsValid(ActiveCamera))
	{
		InitialInvestigationCameraRotation = ActiveCamera->GetRelativeRotation();

		CurrentRotationOffset = FRotator::ZeroRotator;

		ActiveCameraRotation = InitialInvestigationCameraRotation;
	}

	bIgnoreFirstInput = true;

	OnInvestigationViewTargetChanged.Broadcast(NewTarget);
}

FVector UZWInteractionSubsystem::GetCameraLocation()
{
    if (IsValid(ActiveCamera) && ActiveCamera != nullptr)
	{
		//return ActiveCamera->GetActorLocation();
		return ActiveCamera->GetComponentLocation();
	}
	return FVector::ZeroVector;
}

FRotator UZWInteractionSubsystem::GetCameraRotation()
{
	if (IsValid(ActiveCamera) && ActiveCamera != nullptr)
	{
		//return ActiveCamera->GetActorRotation();
		return ActiveCamera->GetComponentRotation();
	}
    return FRotator::ZeroRotator;
}

void UZWInteractionSubsystem::UpdateCameraRotation(FVector2D LookAxisVector)
{
	if (bIgnoreFirstInput)
	{
		bIgnoreFirstInput = false;
		return;
	}
	
	if (IsValid(ActiveCamera))
	{
		CurrentRotationOffset.Yaw += LookAxisVector.X;
		CurrentRotationOffset.Pitch -= LookAxisVector.Y;
		CurrentRotationOffset.Roll = 0;

		CurrentRotationOffset.Pitch = FMath::Clamp(CurrentRotationOffset.Pitch, -89.0f, 89.0f);

		const FRotator NewRotation = InitialInvestigationCameraRotation + CurrentRotationOffset;
		
		ActiveCamera->SetRelativeRotation(NewRotation);
		UE_LOG(LogTemp, Log, TEXT("%f, %f, %f"), InitialInvestigationCameraRotation.Pitch, InitialInvestigationCameraRotation.Yaw, InitialInvestigationCameraRotation.Roll)
		UE_LOG(LogTemp, Log, TEXT("%f, %f, %f"), CurrentRotationOffset.Pitch, CurrentRotationOffset.Yaw, CurrentRotationOffset.Roll)
		UE_LOG(LogTemp, Log, TEXT("%f, %f, %f"), NewRotation.Pitch, NewRotation.Yaw, NewRotation.Roll)
		
		ActiveCameraRotation = NewRotation;
	}
}

void UZWInteractionSubsystem::StartInspection()
{
	SetInteractedObject(GetInteractableObject());
	ResetInteractableObject();
	
	TObjectPtr<AActor> InActor = GetInteractedObject()->GetOwner();
	
	if (IsValid(InActor) && InActor != nullptr)
	{
		FActorSpawnParameters params;
		params.Template = InActor;
		UClass* ItemClass = InActor->GetClass();

		StartInspection(ItemClass, params, GetInteractedObject()->IsRotatable());
	}	
}

void UZWInteractionSubsystem::StartInspection(UClass* ItemClass, const FActorSpawnParameters& InActorParams, bool IsRotatable)
{
	if (bIsPlayerInspecting) { return; }
	
	UWorld* World = GetWorld();
	AActor* const NewActor = World->SpawnActor<AActor>(ItemClass, InActorParams);
	ClonedInspectedActor = NewActor;
	
	if (IsValid(ClonedInspectedActor) && ClonedInspectedActor != nullptr && InteractionSceneCapture != nullptr)
	{
		ClonedInspectedActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
		
		InteractionSceneCapture->UpdateVisibility(ClonedInspectedActor);
		
		UArrowComponent* ArrowComponent = Cast<UArrowComponent>(ClonedInspectedActor->GetComponentByClass(UArrowComponent::StaticClass()));
		if (ArrowComponent)
		{
			InteractionSceneCapture->SetLookAtRotation(FVector(0, 0, 0));
		}
	}
	
	bIsPlayerInspecting = true;
	
	SetInputMode.ExecuteIfBound(Inspection);
	StartInspectionDelegate.Broadcast(IsRotatable);
}

void UZWInteractionSubsystem::AdjustInspectionRotation(FRotator Rotation)
{
	InteractionSceneCapture->AddInspectedActorRotation(Rotation);
}

void UZWInteractionSubsystem::EndInspection()
{
	if (!bIsPlayerInspecting) { return; }
	SetInteractableObject(GetInteractedObject());
	if (bIsPlayerInvestigating && !InvestigatedObjects.IsEmpty())
	{
		SetInteractedObject(InvestigatedObjects.Last());
	}
	else
	{
		ResetInteractedObject();
	}
	InteractionSceneCapture->UpdateVisibility(nullptr);
	if (IsValid(ClonedInspectedActor))
	{
		ClonedInspectedActor->Destroy();
		ClonedInspectedActor = nullptr;
	}
	
	bIsPlayerInspecting = false;
	
	//InspectionWidget->InspectionWidgetEndInspectionDelegate.Unbind();
	if (bIsPlayerInvestigating)
	{
		SetInputMode.ExecuteIfBound(Investigation);
	}
	else
	{
		SetInputMode.ExecuteIfBound(NoInteraction);
	}
	EndInspectionDelegate.Broadcast();

	//if (UProjectXUIManager* UIManager = GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UProjectXUIManager>())
	//{
	//	UIManager->RequestGameInputMode();
	//}
}

void UZWInteractionSubsystem::ResolveInvestigation(AActor* NewTarget)
{
	if (IsValid(NewTarget))
	{
		if (!bIsPlayerInvestigating)
		{
			StartInvestigation(NewTarget);
			SetCamera(NewTarget);
		}
		else if (!InvestigatedObjects.Contains(GetInteractableObject()))
		{
			SetInteractedObject(GetInteractableObject());
			InvestigatedObjects.AddUnique(GetInteractedObject());
			ResetInteractableObject();
			SetCamera(NewTarget);
		}
	}
	else
	{
		if (InvestigatedObjects.Num() <= 1)
		{
			EndInvestigation();
		}
		else
		{
			if (IsValid(InteractedObject))
			{
				InvestigatedObjects.Remove(InteractedObject);
				SetInteractedObject(InvestigatedObjects.Last());
				ResetInteractableObject();
				SetCamera(InteractedObject->GetOwner());
			}
		}
	}
}

bool UZWInteractionSubsystem::IsInvestigatedObject(AActor* InObject)
{
	UActorComponent* ActorComponent = InObject->GetComponentByClass(UZWInteractionComponent::StaticClass());

	if (ActorComponent == nullptr) { return false; }

	UZWInteractionComponent* InteractionComponent = Cast<UZWInteractionComponent>(ActorComponent);

	if (InteractionComponent == nullptr) { return false; }

	if (InvestigatedObjects.Contains(InteractionComponent))
	{
		return true;
	}

	return false;
}

void UZWInteractionSubsystem::StartInvestigation(AActor* InitialTarget)
{
	//if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("Start Investigation")); }
	//SetInputMode.ExecuteIfBound(Investigation);
	SetInteractedObject(GetInteractableObject());
	InvestigatedObjects.AddUnique(GetInteractedObject());
	ResetInteractableObject();
	
	OnInvestigationStarted.Broadcast(InitialTarget);
	
	bIsPlayerInvestigating = true;
}

void UZWInteractionSubsystem::EndInvestigation()
{
	APlayerController* PC = GetWorld()->GetFirstPlayerController();
	if (!PC) return;

	if (IsValid(PC->GetCharacter()) && PC->GetCharacter() != nullptr)
	{
		SetCamera(PC->GetCharacter());
	}
	
	bIsPlayerInvestigating = false;
	////if (GEngine) { GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Red, TEXT("End Investigation")); }
	//SetInputMode.ExecuteIfBound(EInteractionInputMode::NoInteraction);
	InvestigatedObjects.Empty();
	ResetInteractableObject();
	ResetInteractedObject();

	OnInvestigationEnded.Broadcast();
}
*/
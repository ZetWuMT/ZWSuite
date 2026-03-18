// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZWInteractionSubsystem.generated.h"

class AZWInteractionSceneCapture;
class UZWInteractionComponent;
class USceneComponent;

UENUM()
enum EInteractionInputMode : uint8
{
	NoInteraction,
	Inspection,
	Interface,
	Investigation
};

DECLARE_DELEGATE_OneParam(FSetInputModeDelegate, EInteractionInputMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FStartInspectionDelegate, bool);
DECLARE_MULTICAST_DELEGATE(FEndInspectionDelegate);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInvestigationStarted, AActor*);
DECLARE_MULTICAST_DELEGATE(FOnInvestigationEnded);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnInvestigationViewTargetChanged, AActor*);

/**
 * 
 */
UCLASS()
class ZWINTERACTION_API UZWInteractionSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()		

public:
	FSetInputModeDelegate SetInputMode;
	
	FStartInspectionDelegate StartInspectionDelegate;
	FEndInspectionDelegate EndInspectionDelegate;

	FOnInvestigationStarted OnInvestigationStarted;
	FOnInvestigationEnded OnInvestigationEnded;
	FOnInvestigationViewTargetChanged OnInvestigationViewTargetChanged;

	UFUNCTION(BlueprintCallable)
	bool IsPlayerInteracting() { return bIsPlayerInteracting; }
	UFUNCTION(BlueprintCallable)
	bool IsPlayerInspecting() { return bIsPlayerInspecting; }
	UFUNCTION(BlueprintCallable)
	bool IsPlayerInvestigating() { return bIsPlayerInvestigating; }

	TObjectPtr<UZWInteractionComponent> GetInteractableObject() { return InteractableObject; }
	void SetInteractableObject(TObjectPtr<UZWInteractionComponent> Object);
	void ResetInteractableObject();

	TObjectPtr<UZWInteractionComponent> GetInteractedObject() { return InteractedObject; }
	void SetInteractedObject(TObjectPtr<UZWInteractionComponent> Object);
	void ResetInteractedObject();

	TObjectPtr<AZWInteractionSceneCapture> GetInteractionSceneCapture() { return InteractionSceneCapture; }

	void SpawnInteractionSceneCapture();
	void DestroyInteractionSceneCapture();

	void SetCamera(AActor* NewTarget);
	FVector GetCameraLocation();
	FRotator GetCameraRotation();
	void UpdateCameraRotation(FVector2D LookAxisVector);

	UFUNCTION(BlueprintCallable)
	FRotator GetInitialCameraRotation() { return InitialInvestigationCameraRotation; }
	UFUNCTION(BlueprintCallable)
	FRotator GetOffsetRotation() { return CurrentRotationOffset; }
	
	UFUNCTION(BlueprintCallable, Category="Interaction System")
	void StartInspection();

	void StartInspection(UClass* ItemClass, const FActorSpawnParameters& InActorParams, bool IsRotatable);

	UFUNCTION(BlueprintCallable, Category="Interaction System")
	void AdjustInspectionRotation(FRotator Rotation);
	
	UFUNCTION(BlueprintCallable, Category = "Interaction System")
	void EndInspection();
	
	UFUNCTION(BlueprintCallable, Category = "Interaction System")
	void ResolveInvestigation(AActor* NewTarget);

	bool IsInvestigatedObject(AActor* InObject);

	UFUNCTION(BlueprintCallable, Category = "Interaction System")
	void StartInvestigation(AActor* InitialTarget);

	UFUNCTION(BlueprintCallable, Category = "Interaction System")
	void EndInvestigation();

private:
	UPROPERTY()
	TObjectPtr<AActor> ClonedInspectedActor;
	
	FRotator InitialInvestigationCameraRotation;

	FRotator CurrentRotationOffset;

	bool bIgnoreFirstInput = false;

	bool bIsPlayerInteracting = false;

	bool bIsPlayerInvestigating = false;

	bool bIsPlayerInspecting = false;

	TObjectPtr<UZWInteractionComponent> InteractableObject = nullptr;

	TObjectPtr<UZWInteractionComponent> InteractedObject = nullptr;

	TObjectPtr<AZWInteractionSceneCapture> InteractionSceneCapture;

	TArray<TObjectPtr<UZWInteractionComponent>> InvestigatedObjects;

	USceneComponent* ActiveCamera;

	//TODO: I need to pass the information about the parent camera to the subsystem somehow
	AActor* ParentCamera = nullptr;

	FRotator ActiveCameraRotation;
};

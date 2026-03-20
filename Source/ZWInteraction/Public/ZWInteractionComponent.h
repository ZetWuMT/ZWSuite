// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "GameplayTagContainer.h"
#include "ZWInteractionComponent.generated.h"

class UZWInteractionSubsystem;
class UCameraComponent;
class UArrowComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnRegisterActor, FGuid, ActorGuid);

UCLASS(meta = (BlueprintSpawnableComponent))
class ZWINTERACTION_API UZWInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZWInteractionComponent();
	
	UPROPERTY(BlueprintReadOnly)
	FRotator InitialCameraRotation;

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	
	// TODO: To be removed completely from the plugin!
	//UFUNCTION(BlueprintCallable)
	//virtual void NotifyFlowGraph();

	UFUNCTION(BlueprintCallable)
	void Interact();

	//void Inspect();

	//void Investigate();

	void ToggleHighlight(bool bIsHighlighted);

	//bool IsInspectable() { return bIsInspectable; }

	//bool IsInvestigatable() { return bIsInvestigatable; }

	//bool IsInvestigationExclusive() { return bInvestigationExclusive; }

	bool IsHighlighted() { return bIsHighlighted; }

	//bool IsRotatable() { return bIsRotatable; }

	void BPToggleHighlight_Implementation(bool bIsHighlighted);

	//FGuid GetActorGuid() { return ActorGuid; }

	//void DestroyInteractiveActor();

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractDelegate);

	UPROPERTY(BlueprintAssignable, Category="InteractionSystem")
	FOnInteractDelegate OnInteract;

	FOnRegisterActor OnRegisterActor;

	//AActor* ResolveInvestigationParentActor();

#if WITH_EDITOR
	//void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	
	//UFUNCTION(CallInEditor, Category = "InteractionSystem")
	//void CreateCamera();

	//UFUNCTION(CallInEditor, Category = "InteractionSystem")
	//void RemoveCamera();

	//void CreateArrow();

	//void RemoveArrow();
#endif

	//UCameraComponent* GetCameraComponent();

	//FRotator GetInspectionRotationAdjustment();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:
	//UPROPERTY(EditAnywhere, Category = "InteractionSystem")
	//bool bIsInspectable = false;

	//UPROPERTY(EditAnywhere, Category = "InteractionSystem")
	//bool bIsInvestigatable = false;

	//UPROPERTY(EditAnywhere, Category = "InteractionSystem|Inspection Settings", meta = (EditCondition="bIsInspectable", EditConditionHides))
	//bool bIsRotatable = true;

	//UPROPERTY(EditAnywhere, Category = "InteractionSystem|Inspection Settings", meta = (EditCondition="bIsInspectable", EditConditionHides))
	//FRotator InspectionRotationAdjustment;

	//UPROPERTY(VisibleAnywhere, Category = "InteractionSystem|Inspection Settings", meta = (EditCondition="bIsInspectable", EditConditionHides))
	//UArrowComponent* InspectionArrowComponent;
	
	//UPROPERTY(EditAnywhere, Category = "InteractionSystem|Investigation Settings")
	//bool bInvestigationExclusive = false;

	//UPROPERTY(EditAnywhere, Category = "InteractionSystem|Investigation Settings")
	//bool bUseParentForInvestigation = true;

	//UPROPERTY(EditAnywhere, Category = "InteractionSystem|Investigation Settings", meta = (EditCondition="!bUseParentForInvestigation", EditConditionHides))
	//AActor*  InvestigationParentActor = nullptr;

	//UPROPERTY(VisibleAnywhere, Category = "InteractionSystem|Investigation Settings")
	//UCameraComponent* InvestigationCameraComponent;

	bool bIsHighlighted = false;

	FGuid ActorGuid;

	UZWInteractionSubsystem* InteractionSubsystem;

	UStaticMeshComponent* StaticMeshComponent;

	//FVector CameraPosition = FVector(100, 0, 0);
	//FRotator CameraRotation = FRotator(0, 180, 0);
	//FRotator ArrowRotation = FRotator(0,0,0);
};

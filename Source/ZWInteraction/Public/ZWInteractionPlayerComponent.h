// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ArrowComponent.h"
#include "ZWInteractionPlayerComponent.generated.h"

struct FInputActionValue;
struct FGameplayTag;
class UInputAction;
class UZWInteractionSubsystem;
class UZWInteractionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZWINTERACTION_API UZWInteractionPlayerComponent : public UArrowComponent
{
	GENERATED_BODY()



public:	
	// Sets default values for this component's properties
	UZWInteractionPlayerComponent();
	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	void DetectInteractiveObjects();
	void ResolveInteractiveObject(AActor* NewInteractableObject);
	
	void Interact();
	//void EndInvestigation();
	//void InvestigationUpperLayer();

	//bool IsInspecting();
	//bool IsInvestigating();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;	
	
	virtual UZWInteractionComponent* GetInteractableObjectInteractionComponent(AActor* InteractableObject);
	
private:
	UPROPERTY()
	UZWInteractionSubsystem* InteractionSubsystem;
	
	//void SetInteractionDetector(UObject* NewDetector);
};
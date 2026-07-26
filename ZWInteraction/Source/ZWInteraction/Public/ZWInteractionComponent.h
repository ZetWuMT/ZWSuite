// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/SceneComponent.h"
#include "ZWInteractionComponent.generated.h"

UCLASS(meta = (BlueprintSpawnableComponent))
class ZWINTERACTION_API UZWInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZWInteractionComponent();
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Interact();

	/**
	 * This function determines how the game is highlighting the interactable object.
	 * By default, the object is highlighted by applying a post process material of a user's choice. 
	 * Override this function to set up the way you are providing the visual cue for the interactable object.
	 * @param bIsHighlighted 
	 */
	UFUNCTION(Blueprintable)
	virtual void ToggleHighlight(bool bIsHighlighted);

	bool IsHighlighted() const { return bIsHighlighted; }

	DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractDelegate);

	/**
	 * The event called when the object is interacted with in any way.
	 */
	UPROPERTY(BlueprintAssignable, Category="InteractionSystem")
	FOnInteractDelegate OnInteract;

private:
	UFUNCTION(BlueprintCallable)
	void AddStaticMeshesToHighlightPool(TArray<UStaticMeshComponent*> Components);
	
	UFUNCTION(BlueprintCallable)
	void AddSkeletalMeshesToHighlightPool(TArray<USkeletalMeshComponent*> Components);
	
protected:
	bool bIsHighlighted = false;

private:
	UPROPERTY(Transient)
	UStaticMeshComponent* StaticMeshComponent;
	
	UPROPERTY(Transient)
	USkeletalMeshComponent* SkeletalMeshComponent;
	
	UPROPERTY(Transient)
	TArray<UStaticMeshComponent*> AdditionalStaticMeshComponents;
	
	UPROPERTY(Transient)
	TArray<USkeletalMeshComponent*> AdditionalSkeletalMeshComponents;
};

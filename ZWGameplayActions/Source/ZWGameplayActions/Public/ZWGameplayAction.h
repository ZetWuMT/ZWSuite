// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "UObject/Object.h"
#include "ZWGameplayAction.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, Abstract, EditInlineNew)
class ZWGAMEPLAYACTIONS_API UZWGameplayAction : public UObject
{
	GENERATED_BODY()
	
public:
	// Tag that triggers this action (e.g. Input.Action.Interact)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Config")
	FGameplayTag TriggerTag;

	// Main logic function - overridden in Blueprints
	UFUNCTION(BlueprintNativeEvent, Category = "Action Execution")
	void ExecuteAction(APlayerController* Controller, APawn* Pawn);

	// Allows using nodes with a world context (e.g. LineTrace, SpawnActor) in Blueprint
	virtual UWorld* GetWorld() const override;
};

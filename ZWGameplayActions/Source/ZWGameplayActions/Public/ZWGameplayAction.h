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
	// Tag, który wyzwala tę akcję (np. Input.Action.Interact)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Action Config")
	FGameplayTag TriggerTag;

	// Główna funkcja logiki - nadpisywana w Blueprintach
	UFUNCTION(BlueprintNativeEvent, Category = "Action Execution")
	void ExecuteAction(APlayerController* Controller, APawn* Pawn);

	// Pozwala używać nodów z kontekstem świata (np. LineTrace, SpawnActor) w Blueprincie
	virtual UWorld* GetWorld() const override;
};

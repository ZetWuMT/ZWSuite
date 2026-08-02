// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "InputTriggers.h"
#include "ZWInputConfig.generated.h"

class UInputAction;

// 1. Structure of a single mapping (Action -> Tag)
USTRUCT(BlueprintType)
struct FZWInputAction
{
	GENERATED_BODY()

	// Physical action from Enhanced Input (e.g. IA_ToggleInventory)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UInputAction> InputAction;

	// Logical tag to be sent after pressing (e.g. "UI.Action.Inventory")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	ETriggerEvent TriggerEvent = ETriggerEvent::Started;
};

/**
 * 
 */
UCLASS()
class ZWCORE_API UZWInputConfig : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Native Input", meta = (TitleProperty = "InputAction"))
	TArray<FZWInputAction> NativeInputActions;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Generic Input", meta = (TitleProperty = "InputAction"))
	TArray<FZWInputAction> GenericInputActions;
};

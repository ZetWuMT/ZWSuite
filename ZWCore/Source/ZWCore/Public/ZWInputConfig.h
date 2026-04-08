// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ZWInputConfig.generated.h"

class UInputAction;

// 1. Struktura pojedynczego mapowania (Akcja -> Tag)
USTRUCT(BlueprintType)
struct FZWInputAction
{
	GENERATED_BODY()

	// Fizyczna akcja z Enhanced Input (np. IA_ToggleInventory)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSoftObjectPtr<UInputAction> InputAction;

	// Logiczny tag, który ma zostać wysłany po wciśnięciu (np. "UI.Action.Inventory")
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (Categories = "InputTag"))
	FGameplayTag InputTag;
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

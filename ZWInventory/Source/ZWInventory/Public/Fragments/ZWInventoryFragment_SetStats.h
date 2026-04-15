// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZWInventoryItemDefinition.h"
#include "ZWInventoryFragment_SetStats.generated.h"

class UZWInventoryItemInstance;
class UObject;

/**
 * 
 */
UCLASS()
class ZWINVENTORY_API UZWInventoryFragment_SetStats : public UZWInventoryItemFragment
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UZWInventoryItemInstance* Instance) const override;

	int32 GetItemStatByTag(FGameplayTag Tag) const;
	
protected:
	UPROPERTY(EditDefaultsOnly, Category=Equipment)
	TMap<FGameplayTag, int32> InitialItemStats;
};

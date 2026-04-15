// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "GameplayTagStack.h"
#include "ZWInventoryItemDefinition.h"
#include "Templates/SubclassOf.h"

#include "ZWInventoryItemInstance.generated.h"

class FLifetimeProperty;

class UZWInventoryItemDefinition;
class UZWInventoryItemFragment;
struct FFrame;
struct FGameplayTag;

UENUM()
enum class EHasFragment : uint8
{
	Valid,
	NotValid
};

USTRUCT()
struct FZWInventoryItemInstanceSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FZWInventoryItemDefinitionSaveData InventoryItemDefinitionSaveData;

	UPROPERTY(SaveGame)
	TArray<uint8> Data;
	
};

/**
 * UInventoryItemInstance
 */
UCLASS(BlueprintType)
class ZWINVENTORY_API UZWInventoryItemInstance : public UObject
{
	GENERATED_BODY()

public:
	UZWInventoryItemInstance(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	// Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory)
	void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
	UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory)
	void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

	// Returns the stack count of the specified tag (or 0 if the tag is not present)
	UFUNCTION(BlueprintCallable, Category=Inventory)
	int32 GetStatTagStackCount(FGameplayTag Tag) const;

	// Returns true if there is at least one stack of the specified tag
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool HasStatTag(FGameplayTag Tag) const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Stacking")
	int32 GetStackCount() const;
	
	UFUNCTION(BlueprintPure, Category = "Inventory|Stacking")
	int32 GetMaxStackCount() const;

	UFUNCTION(BlueprintPure, Category = "Inventory|Stacking")
	int32 GetTotalStackCount() const;
	
	TSoftObjectPtr<UZWInventoryItemDefinition> GetItemDef() const { return ItemDef; }
	
	UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass))
	const UZWInventoryItemFragment* FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const;

	template <typename ResultClass>
	const ResultClass* FindFragmentByClass() const
	{
		return (ResultClass*)FindFragmentByClass(ResultClass::StaticClass());
	}

	UFUNCTION(BlueprintCallable, Category=Inventory)
	TSoftObjectPtr<UZWInventoryItemDefinition> GetItemDefinition() {return ItemDef;}	

	void SaveItemInstance(FZWInventoryItemInstanceSaveData& SaveData);

	void LoadItemInstance(FZWInventoryItemInstanceSaveData& SaveData);

private:

	void SetItemDef(TSoftObjectPtr<UZWInventoryItemDefinition> InDef);

	friend struct FZWInventoryList;

	UPROPERTY()
	FGameplayTagStackContainer StatTags;

	// The item definition
	UPROPERTY()
	TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef;	
};

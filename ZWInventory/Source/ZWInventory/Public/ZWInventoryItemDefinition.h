// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWInventoryItemDefinition.generated.h"

template <typename T> class TSubclassOf;

class UZWInventoryItemInstance;
struct FFrame;

USTRUCT()
struct FZWInventoryItemFragmentSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	UClass* FragmentClass;
	
	UPROPERTY(SaveGame)
	TArray<uint8> Data;

	FZWInventoryItemFragmentSaveData()
	{
		FragmentClass = nullptr;
	}
};

USTRUCT()
struct FZWInventoryItemDefinitionSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	UClass* Class;
	
	UPROPERTY(SaveGame)
	TArray<FZWInventoryItemFragmentSaveData> FragmentRecords;	

	UPROPERTY(SaveGame)
	TArray<uint8> Data;

	FZWInventoryItemDefinitionSaveData()
	{
		Class = nullptr;
	}
};

//////////////////////////////////////////////////////////////////////

// Represents a fragment of an item definition
UCLASS(DefaultToInstanced, EditInlineNew, Abstract)
class ZWINVENTORY_API UZWInventoryItemFragment : public UObject
{
	GENERATED_BODY()

public:
	virtual void OnInstanceCreated(UZWInventoryItemInstance* Instance) const {}

	virtual void SaveFragment(FZWInventoryItemDefinitionSaveData& InventoryItemDefinitionSaveData) {}

	virtual void LoadFragment(FZWInventoryItemFragmentSaveData& InventoryItemFragmentSaveData) {}
};

/**
 * UInventoryItemDefinition
 */
UCLASS(Blueprintable, Const, EditInlineNew)
class ZWINVENTORY_API UZWInventoryItemDefinition : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UZWInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display)
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, Instanced)
	TArray<TObjectPtr<UZWInventoryItemFragment>> Fragments;

	void SaveDefinition(FZWInventoryItemDefinitionSaveData& InventoryItemDefinitionSaveData);

	void LoadDefinition(FZWInventoryItemDefinitionSaveData& InventoryItemDefinitionSaveData);

public:
	const UZWInventoryItemFragment* FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const;
	
	template <typename FragmentClass>
	const FragmentClass* FindFragmentByClass() const
	{
		const UZWInventoryItemFragment* FoundFragment = FindFragmentByClass(FragmentClass::StaticClass());
		return Cast<FragmentClass>(FoundFragment);
	}
};

//@TODO: Make into a subsystem instead?
UCLASS()
class UZWInventoryFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass))
	static const UZWInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UZWInventoryItemDefinition> ItemDef, TSubclassOf<UZWInventoryItemFragment> FragmentClass);
};
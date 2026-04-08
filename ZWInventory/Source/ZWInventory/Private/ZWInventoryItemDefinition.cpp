// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWInventoryItemDefinition.h"

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Templates/SubclassOf.h"
#include "UObject/ObjectPtr.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZWInventoryItemDefinition)

UZWInventoryItemDefinition::UZWInventoryItemDefinition(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZWInventoryItemDefinition::SaveDefinition(FZWInventoryItemDefinitionSaveData& InventoryItemDefinitionSaveData)
{
	for (TObjectPtr<UZWInventoryItemFragment>& ItemFragment : Fragments)
	{
		FZWInventoryItemFragmentSaveData ItemFragmentSaveData;
		ItemFragment->SaveFragment(InventoryItemDefinitionSaveData);
		//InventoryItemDefinitionSaveData.FragmentRecords.Emplace(ItemFragmentSaveDataSaveData);
	}
	
	FMemoryWriter MemWriter(InventoryItemDefinitionSaveData.Data, true);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);
}

void UZWInventoryItemDefinition::LoadDefinition(FZWInventoryItemDefinitionSaveData& InventoryItemDefinitionSaveData)
{
	FMemoryReader MemReader(InventoryItemDefinitionSaveData.Data);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);

	for (FZWInventoryItemFragmentSaveData FragmentData : InventoryItemDefinitionSaveData.FragmentRecords)
	{
		UE_LOG(LogTemp, Log, TEXT("%s"), *FragmentData.StaticStruct()->GetName())
		//auto temp = FragmentData.FragmentClass->StaticClass();
		if (UZWInventoryItemFragment* ItemFragment = NewObject<UZWInventoryItemFragment>(this, FragmentData.FragmentClass))
		{
			FragmentData.StaticStruct();
			ItemFragment->LoadFragment(FragmentData);
			Fragments.Emplace(ItemFragment);
		}
	}
}

const UZWInventoryItemFragment* UZWInventoryItemDefinition::FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const
{
	if (FragmentClass != nullptr)
	{
		for (UZWInventoryItemFragment* Fragment : Fragments)
		{
			if (Fragment && Fragment->IsA(FragmentClass))
			{
				return Fragment;
			}
		}
	}

	return nullptr;
}

const UZWInventoryItemFragment* UZWInventoryFunctionLibrary::FindItemDefinitionFragment(TSubclassOf<UZWInventoryItemDefinition> ItemDef, TSubclassOf<UZWInventoryItemFragment> FragmentClass)
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		return GetDefault<UZWInventoryItemDefinition>(ItemDef)->FindFragmentByClass(FragmentClass);
	}
	return nullptr;
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryItemInstance.h"

#include "ZWInventoryItemDefinition.h"
//#include "Net/UnrealNetwork.h"

//#if UE_WITH_IRIS
//#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
//#endif // UE_WITH_IRIS

#include "ZWInventorySettings.h"
#include "Fragments/ZWInventoryFragment_SetStats.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZWInventoryItemInstance)

class FLifetimeProperty;

UZWInventoryItemInstance::UZWInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UZWInventoryItemInstance::AddStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.AddStack(Tag, StackCount);
}

void UZWInventoryItemInstance::RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)
{
	StatTags.RemoveStack(Tag, StackCount);
}

int32 UZWInventoryItemInstance::GetStatTagStackCount(FGameplayTag Tag) const
{
	return StatTags.GetStackCount(Tag);
}

bool UZWInventoryItemInstance::HasStatTag(FGameplayTag Tag) const
{
	return StatTags.ContainsTag(Tag);
}

int32 UZWInventoryItemInstance::GetStackCount() const
{
	const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
	
	// If stacking is disabled globally, every item is treated as a single instance (count = 1)
	if (!Settings->bEnableStacking || !Settings->StackCountTag.IsValid())
	{
		return 1;
	}

	// Current stack count is dynamic, so we check the Instance's StatTagStack.
	// We return at least 1, assuming the item exists.
	return FMath::Max(1, GetStatTagStackCount(Settings->StackCountTag));
}

int32 UZWInventoryItemInstance::GetMaxStackCount() const
{
	const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
	
	if (!Settings->bEnableStacking || !Settings->MaxStackCountTag.IsValid())
	{
		return 1;
	}

	// Max Stack Count is static, so we look for it in the Item Definition's fragment
	if (const UZWInventoryFragment_SetStats* StatFragment = FindFragmentByClass<UZWInventoryFragment_SetStats>())
	{
		// Look up the value mapped to the MaxStackCountTag defined in project settings
		int32 FoundMax = StatFragment->GetItemStatByTag(Settings->MaxStackCountTag);
		
		// If the tag exists in the map and is greater than 0, return it. Otherwise default to 1.
		return FoundMax > 0 ? FoundMax : 1;
	}

	return 1;
}

int32 UZWInventoryItemInstance::GetTotalStackCount() const
{
	const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
	
	if (!Settings->bEnableStacking || !Settings->TotalStackCountTag.IsValid())
	{
		// -1 could represent "infinity" / "no limit"
		return -1; 
	}

	if (const UZWInventoryFragment_SetStats* StatFragment = FindFragmentByClass<UZWInventoryFragment_SetStats>())
	{
		return StatFragment->GetItemStatByTag(Settings->TotalStackCountTag);
	}

	return -1;
}

void UZWInventoryItemInstance::SaveItemInstance(FZWInventoryItemInstanceSaveData& SaveData)
{
	FZWInventoryItemDefinitionSaveData ItemDefinitionSaveData;

	ItemDefinitionSaveData.Class = ItemDef->GetClass();
	
	UZWInventoryItemDefinition* LoadedItemDef = ItemDef.LoadSynchronous();
	if (IsValid(LoadedItemDef))
	{
		return LoadedItemDef->SaveDefinition(ItemDefinitionSaveData);
	}
	SaveData.InventoryItemDefinitionSaveData = ItemDefinitionSaveData;
	
	FMemoryWriter MemWriter(SaveData.Data, true);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);
}

void UZWInventoryItemInstance::LoadItemInstance(FZWInventoryItemInstanceSaveData& SaveData)
{
	FMemoryReader MemReader(SaveData.Data);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);
	
	UZWInventoryItemDefinition* ItemDefinition = NewObject<UZWInventoryItemDefinition>();
	ItemDefinition->LoadDefinition(SaveData.InventoryItemDefinitionSaveData);
}

void UZWInventoryItemInstance::SetItemDef(TSoftObjectPtr<UZWInventoryItemDefinition> InDef)
{
	ItemDef = InDef;
}

const UZWInventoryItemFragment* UZWInventoryItemInstance::FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const
{
	if ((ItemDef != nullptr) && (FragmentClass != nullptr))
	{
		UZWInventoryItemDefinition* LoadedItemDef = ItemDef.LoadSynchronous();
		if (!IsValid(LoadedItemDef)) return nullptr;
		{
			return LoadedItemDef->FindFragmentByClass(FragmentClass);
		}		
	}

	return nullptr;
}
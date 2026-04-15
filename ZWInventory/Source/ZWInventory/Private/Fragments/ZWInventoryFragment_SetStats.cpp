// Fill out your copyright notice in the Description page of Project Settings.

#include "Fragments/ZWInventoryFragment_SetStats.h"
#include "ZWInventoryItemInstance.h"

void UZWInventoryFragment_SetStats::OnInstanceCreated(UZWInventoryItemInstance* Instance) const
{
	for (const auto& KVP : InitialItemStats)
	{
		Instance->AddStatTagStack(KVP.Key, KVP.Value);
	}
}

int32 UZWInventoryFragment_SetStats::GetItemStatByTag(FGameplayTag Tag) const
{
	if (const int32* StatPtr = InitialItemStats.Find(Tag))
	{
		return *StatPtr;
	}

	return 0;
}
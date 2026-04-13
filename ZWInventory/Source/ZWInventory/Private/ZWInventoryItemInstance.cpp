// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryItemInstance.h"

#include "ZWInventoryItemDefinition.h"
//#include "Net/UnrealNetwork.h"

//#if UE_WITH_IRIS
//#include "Iris/ReplicationSystem/ReplicationFragmentUtil.h"
//#endif // UE_WITH_IRIS

#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZWInventoryItemInstance)

class FLifetimeProperty;

UZWInventoryItemInstance::UZWInventoryItemInstance(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}
/*
void UInventoryItemInstance::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	//DOREPLIFETIME(ThisClass, StatTags);
	//DOREPLIFETIME(ThisClass, ItemDef);
}

#if UE_WITH_IRIS
void UInventoryItemInstance::RegisterReplicationFragments(UE::Net::FFragmentRegistrationContext& Context, UE::Net::EFragmentRegistrationFlags RegistrationFlags)
{
	using namespace UE::Net;

	// Build descriptors and allocate PropertyReplicationFragments for this object
	FReplicationFragmentUtil::CreateAndRegisterFragmentsForObject(this, Context, RegistrationFlags);
}
#endif // UE_WITH_IRIS
*/
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

void UZWInventoryItemInstance::SaveItemInstance(FZWInventoryItemInstanceSaveData& SaveData)
{
	FZWInventoryItemDefinitionSaveData ItemDefinitionSaveData;

	ItemDefinitionSaveData.Class = ItemDef->GetClass();

	// Cannot be DefaultObject
	UZWInventoryItemDefinition* LoadedItemDef = ItemDef.LoadSynchronous();
	if (IsValid(LoadedItemDef))
	{
		return LoadedItemDef->SaveDefinition(ItemDefinitionSaveData);
	}
	//ItemDef->GetDefaultObject<UInventoryItemDefinition>()->SaveDefinition(ItemDefinitionSaveData);
	//UInventoryItemDefinition* ItemDefinition = Cast<UInventoryItemDefinition>(ItemDef);
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

	//UInventoryItemDefinition* ItemDefinition = NewObject<UInventoryItemDefinition>();
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
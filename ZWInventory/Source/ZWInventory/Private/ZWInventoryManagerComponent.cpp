// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryManagerComponent.h"

#include "Engine/World.h"
#include "ZWInventoryItemDefinition.h"
#include "ZWInventoryItemInstance.h"
#include "ZWInventorySubsystem.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZWInventoryManagerComponent)

//class UEvidenceFragment_EvidenceType;
class FLifetimeProperty;
struct FReplicationFlags;

//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ProjectX_Inventory_Message_StackChanged, "ProjectX.Inventory.Message.StackChanged");

//////////////////////////////////////////////////////////////////////
// FInventoryEntry

FString FZWInventoryEntry::GetDebugString() const
{
	TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef;
	if (Instance != nullptr)
	{
		ItemDef = Instance->GetItemDef();
	}

	UZWInventoryItemDefinition* LoadedItemDef = ItemDef.LoadSynchronous();
	if (!IsValid(LoadedItemDef))
	{
		return FString::Printf(TEXT(""));
	}

	return FString::Printf(TEXT("%s (%d x %s)"), *GetNameSafe(Instance), StackCount, *GetNameSafe(LoadedItemDef));
}

//@TODO: FOr some reason Item Instance doesnt get saved/loaded correctly and results with a crash
void FZWInventoryEntry::SaveEntry(FZWInventoryEntrySaveData& EntrySaveData)
{
	FZWInventoryItemInstanceSaveData InstanceSaveData;
	Instance->SaveItemInstance(InstanceSaveData);
	EntrySaveData.InventoryItemInstanceSaveData = InstanceSaveData;
	EntrySaveData.InstanceClass = Instance->GetClass();
	EntrySaveData.Outer = Instance->GetOuter();
}

void FZWInventoryEntry::LoadEntry(FZWInventoryEntrySaveData& EntrySaveData)
{
	UZWInventoryItemInstance* NewInstance = NewObject<UZWInventoryItemInstance>(EntrySaveData.Outer, EntrySaveData.InstanceClass->StaticClass());
	NewInstance->LoadItemInstance(EntrySaveData.InventoryItemInstanceSaveData);
	Instance = NewInstance;
}

//////////////////////////////////////////////////////////////////////
// FInventoryList

//void FInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
//{
//	for (int32 Index : RemovedIndices)
//	{
//		FInventoryEntry& Stack = Entries[Index];
//		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
//		Stack.LastObservedCount = 0;
//	}
//}
//
//void FInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
//{
//	for (int32 Index : AddedIndices)
//	{
//		FInventoryEntry& Stack = Entries[Index];
//		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
//		Stack.LastObservedCount = Stack.StackCount;
//	}
//}
//
//void FInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
//{
//	for (int32 Index : ChangedIndices)
//	{
//		FInventoryEntry& Stack = Entries[Index];
//		check(Stack.LastObservedCount != INDEX_NONE);
//		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
//		Stack.LastObservedCount = Stack.StackCount;
//	}
//}

void FZWInventoryList::BroadcastChangeMessage(FZWInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FZWInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	//UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	//MessageSystem.BroadcastMessage(TAG_ProjectX_Inventory_Message_StackChanged, Message);
}

UZWInventoryItemInstance* FZWInventoryList::AddEntry(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UZWInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
 	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());	

	FZWInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UZWInventoryItemInstance>(OwnerComponent->GetOwner());  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Instance->SetItemDef(ItemDef);
	
	for (UZWInventoryItemFragment* Fragment : ItemDef->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	Result = NewEntry.Instance;

	//const UInventoryItemDefinition* ItemCDO = GetDefault<UInventoryItemDefinition>(ItemDef);
	//MarkItemDirty(NewEntry);

	return Result;
}

void FZWInventoryList::AddEntry(UZWInventoryItemInstance* Instance)
{
	unimplemented();
}

void FZWInventoryList::RemoveEntry(UZWInventoryItemInstance* Instance)
{
	for (auto EntryIt = Entries.CreateIterator(); EntryIt; ++EntryIt)
	{
		FZWInventoryEntry& Entry = *EntryIt;
		if (Entry.Instance == Instance)
		{
			EntryIt.RemoveCurrent();
			//MarkArrayDirty();
		}
	}
}

void FZWInventoryList::SaveInventoryList(FZWInventoryListSaveData& InventoryListSaveData)
{
	for (FZWInventoryEntry Entry : Entries)
	{
		FZWInventoryEntrySaveData EntrySaveData;
		Entry.SaveEntry(EntrySaveData);
		InventoryListSaveData.EntryRecords.Emplace(EntrySaveData);
	}
}

void FZWInventoryList::LoadInventoryList(FZWInventoryListSaveData& InventoryListSaveData)
{
	for (FZWInventoryEntrySaveData Entry : InventoryListSaveData.EntryRecords)
	{
		FZWInventoryEntry NewEntry;
		UE_LOG(LogTemp, Log, TEXT("%s"), *Entry.InstanceClass->StaticClass()->GetName());
		NewEntry.LoadEntry(Entry);
		Entries.Emplace(NewEntry);
	}
}

TArray<UZWInventoryItemInstance*> FZWInventoryList::GetAllItems() const
{
	TArray<UZWInventoryItemInstance*> Results;
	Results.Reserve(Entries.Num());
	for (const FZWInventoryEntry& Entry : Entries)
	{
		if (Entry.Instance != nullptr) //@TODO: Would prefer to not deal with this here and hide it further?
		{
			Results.Add(Entry.Instance);
		}
	}
	return Results;
}

//////////////////////////////////////////////////////////////////////
// UInventoryManagerComponent

UZWInventoryManagerComponent::UZWInventoryManagerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
	, InventoryList(this)
{
	SetIsReplicatedByDefault(true);
}

void UZWInventoryManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		if (UZWInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UZWInventorySubsystem>())
		{
			InventorySubsystem->RegisterInventory(this);
		}
	}
}

void UZWInventoryManagerComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UGameInstance* GameInstance = GetWorld()->GetGameInstance())
	{
		if (UZWInventorySubsystem* InventorySubsystem = GameInstance->GetSubsystem<UZWInventorySubsystem>())
		{
			InventorySubsystem->UnregisterInventory(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}
/*
void UInventoryManagerComponent::GetLifetimeReplicatedProps(TArray< FLifetimeProperty >& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, InventoryList);
}*/

bool UZWInventoryManagerComponent::CanAddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

UZWInventoryItemInstance* UZWInventoryManagerComponent::AddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UZWInventoryItemInstance* Result = nullptr;
	if (ItemDef != nullptr)
	{
		Result = InventoryList.AddEntry(ItemDef, StackCount);
		
		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && Result)
		{
			AddReplicatedSubObject(Result);
		}
	}
	return Result;
}

void UZWInventoryManagerComponent::AddItemInstance(UZWInventoryItemInstance* ItemInstance)
{
	InventoryList.AddEntry(ItemInstance);
	if (IsUsingRegisteredSubObjectList() && IsReadyForReplication() && ItemInstance)
	{
		AddReplicatedSubObject(ItemInstance);
	}
}

void UZWInventoryManagerComponent::RemoveItemInstance(UZWInventoryItemInstance* ItemInstance)
{
	InventoryList.RemoveEntry(ItemInstance);

	if (ItemInstance && IsUsingRegisteredSubObjectList())
	{
		RemoveReplicatedSubObject(ItemInstance);
	}
}

TArray<UZWInventoryItemInstance*> UZWInventoryManagerComponent::GetAllItems() const
{
	return InventoryList.GetAllItems();
}

UZWInventoryItemInstance* UZWInventoryManagerComponent::FindFirstItemStackByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const
{
	for (const FZWInventoryEntry& Entry : InventoryList.Entries)
	{
		UZWInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				return Instance;
			}
		}
	}

	return nullptr;
}

int32 UZWInventoryManagerComponent::GetTotalItemCountByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const
{
	int32 TotalCount = 0;
	for (const FZWInventoryEntry& Entry : InventoryList.Entries)
	{
		UZWInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance))
		{
			if (Instance->GetItemDef() == ItemDef)
			{
				++TotalCount;
			}
		}
	}

	return TotalCount;
}

bool UZWInventoryManagerComponent::ConsumeItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority())
	{
		return false;
	}

	//@TODO: N squared right now as there's no acceleration structure
	int32 TotalConsumed = 0;
	while (TotalConsumed < NumToConsume)
	{
		if (UZWInventoryItemInstance* Instance = UZWInventoryManagerComponent::FindFirstItemStackByDefinition(ItemDef))
		{
			InventoryList.RemoveEntry(Instance);
			++TotalConsumed;
		}
		else
		{
			return false;
		}
	}

	return TotalConsumed == NumToConsume;
}

FZWInventoryManagerComponentSaveData UZWInventoryManagerComponent::SaveInventoryManager()
{
	FZWInventoryManagerComponentSaveData NewInventoryManagerSaveData;

	for (UZWInventoryItemInstance* ItemInstance : InventoryList.GetAllItems())
	{
		//@TODO: We need to save also fragments
		TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef = ItemInstance->GetItemDef();
		FZWInventoryItemDefinitionSaveData ItemDefinitionSaveData;
		ItemDef->SaveDefinition(ItemDefinitionSaveData);
		//NewInventoryManagerSaveData.DefinitionRecords.Emplace(ItemDefinitionSaveData);
		NewInventoryManagerSaveData.ItemDefinitions.Emplace(ItemDef);
		NewInventoryManagerSaveData.DefinitionRecords.Emplace(ItemDef, ItemDefinitionSaveData);
	}

	//InventoryList.SaveInventoryList(NewInventoryManagerSaveData.InventoryListSaveData);
	
	FMemoryReader MemWriter(NewInventoryManagerSaveData.Data, true);
	FObjectAndNameAsStringProxyArchive Ar(MemWriter, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);

	return NewInventoryManagerSaveData;
}

void UZWInventoryManagerComponent::LoadInventoryManager(FZWInventoryManagerComponentSaveData& InventoryManagerSaveData)
{
	FMemoryReader MemReader(InventoryManagerSaveData.Data);
	FObjectAndNameAsStringProxyArchive Ar(MemReader, true);
	Ar.ArIsSaveGame = true;
	Serialize(Ar);

	for (TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef : InventoryManagerSaveData.ItemDefinitions)
	{
		ItemDef->LoadDefinition(InventoryManagerSaveData.DefinitionRecords[ItemDef]);
		AddItemDefinition(ItemDef);
	}
}

/*
void UInventoryManagerComponent::ReadyForReplication()
{
	Super::ReadyForReplication();

	// Register existing UInventoryItemInstance
	if (IsUsingRegisteredSubObjectList())
	{
		for (const FInventoryEntry& Entry : InventoryList.Entries)
		{
			UInventoryItemInstance* Instance = Entry.Instance;

			if (IsValid(Instance))
			{
				AddReplicatedSubObject(Instance);
			}
		}
	}
}

bool UInventoryManagerComponent::ReplicateSubobjects(UActorChannel* Channel, class FOutBunch* Bunch, FReplicationFlags* RepFlags)
{
	bool WroteSomething = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	for (FInventoryEntry& Entry : InventoryList.Entries)
	{
		UInventoryItemInstance* Instance = Entry.Instance;

		if (Instance && IsValid(Instance))
		{
			WroteSomething |= Channel->ReplicateSubobject(Instance, *Bunch, *RepFlags);
		}
	}

	return WroteSomething;
}*/

//////////////////////////////////////////////////////////////////////
//

// UCLASS(Abstract)
// class UInventoryFilter : public UObject
// {
// public:
// 	virtual bool PassesFilter(UInventoryItemInstance* Instance) const { return true; }
// };

// UCLASS()
// class UInventoryFilter_HasTag : public UInventoryFilter
// {
// public:
// 	virtual bool PassesFilter(UInventoryItemInstance* Instance) const { return true; }
// };

// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryManagerComponent.h"

#include "Engine/World.h"
#include "ZWInventoryItemDefinition.h"
#include "ZWInventoryItemInstance.h"
#include "ZWInventorySettings.h"
#include "ZWInventorySubsystem.h"
#include "Fragments/ZWInventoryFragment_SetStats.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(ZWInventoryManagerComponent)

class FLifetimeProperty;
struct FReplicationFlags;

//UE_DEFINE_GAMEPLAY_TAG_STATIC(TAG_ZWInventory_Message_StackChanged, "Inventory.Message.StackChanged");

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

void FZWInventoryEntry::PreReplicatedRemove(const FZWInventoryList& InArraySerializer)
{
	//InArraySerializer.BroadcastChangeMessage(*this, LastObservedCount, 0);
}

void FZWInventoryEntry::PostReplicatedAdd(const FZWInventoryList& InArraySerializer)
{
	SyncInstanceToEntry();

	// 2. Opcjonalnie: Wyślij wiadomość, że podniesiono nowy przedmiot od zera
	// Stara ilość: 0, Nowa ilość: StackCount
	// InArraySerializer.BroadcastChangeMessage(*this, 0, StackCount);

	// 3. Zapisz obecny stan na przyszłość
	LastObservedCount = StackCount;
}

void FZWInventoryEntry::PostReplicatedChange(const FZWInventoryList& InArraySerializer)
{
	// 1. Zsynchronizuj Tagi na Instancji
	SyncInstanceToEntry();

	// 2. Wyślij wiadomość o różnicy (Delta) do UI!
	// InArraySerializer.BroadcastChangeMessage(*this, LastObservedCount, StackCount);

	// 3. Zapisz nowy stan
	LastObservedCount = StackCount;
}

void FZWInventoryEntry::SyncInstanceToEntry()
{
	if (Instance)
	{
		const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
		if (Settings->bEnableStacking && Settings->StackCountTag.IsValid())
		{
			// Resetujemy tag do zera i ustawiamy nową wartość na sztywno,
			// gwarantując 100% zgodności między Strukturą a Instancją.
			
			int32 CurrentTagCount = Instance->GetStatTagStackCount(Settings->StackCountTag);
			if (CurrentTagCount != StackCount)
			{
				// Usuwamy stary tag (jeśli był)
				Instance->RemoveStatTagStack(Settings->StackCountTag, CurrentTagCount);
				// Wrzucamy nową, prawidłową wartość ze Struktury
				Instance->AddStatTagStack(Settings->StackCountTag, StackCount);
			}
		}
	}
}

//@TODO: For some reason Item Instance doesnt get saved/loaded correctly and results with a crash
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

void FZWInventoryList::PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize)
{
	for (int32 Index : RemovedIndices)
	{
		FZWInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.StackCount, /*NewCount=*/ 0);
		Stack.LastObservedCount = 0;
	}
}

void FZWInventoryList::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
	for (int32 Index : AddedIndices)
	{
		FZWInventoryEntry& Stack = Entries[Index];
		BroadcastChangeMessage(Stack, /*OldCount=*/ 0, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FZWInventoryList::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
	for (int32 Index : ChangedIndices)
	{
		FZWInventoryEntry& Stack = Entries[Index];
		check(Stack.LastObservedCount != INDEX_NONE);
		BroadcastChangeMessage(Stack, /*OldCount=*/ Stack.LastObservedCount, /*NewCount=*/ Stack.StackCount);
		Stack.LastObservedCount = Stack.StackCount;
	}
}

void FZWInventoryList::BroadcastChangeMessage(FZWInventoryEntry& Entry, int32 OldCount, int32 NewCount)
{
	FZWInventoryChangeMessage Message;
	Message.InventoryOwner = OwnerComponent;
	Message.Instance = Entry.Instance;
	Message.NewCount = NewCount;
	Message.Delta = NewCount - OldCount;

	//UGameplayMessageSubsystem& MessageSystem = UGameplayMessageSubsystem::Get(OwnerComponent->GetWorld());
	//MessageSystem.BroadcastMessage(TAG_ZWInventory_Message_StackChanged, Message);
}

UZWInventoryItemInstance* FZWInventoryList::AddEntry(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	UZWInventoryItemInstance* Result = nullptr;

	check(ItemDef != nullptr);
 	check(OwnerComponent);

	AActor* OwningActor = OwnerComponent->GetOwner();
	check(OwningActor->HasAuthority());	

	FZWInventoryEntry& NewEntry = Entries.AddDefaulted_GetRef();
	NewEntry.Instance = NewObject<UZWInventoryItemInstance>(OwningActor);  //@TODO: Using the actor instead of component as the outer due to UE-127172
	NewEntry.Instance->SetItemDef(ItemDef);
	
	for (UZWInventoryItemFragment* Fragment : ItemDef->Fragments)
	{
		if (Fragment != nullptr)
		{
			Fragment->OnInstanceCreated(NewEntry.Instance);
		}
	}
	NewEntry.StackCount = StackCount;
	//NewEntry.SyncInstanceToEntry(); //@TODO: For Multiplayer
	
	MarkItemDirty(NewEntry);
	
	Result = NewEntry.Instance;
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
			MarkArrayDirty();
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
		if (Entry.Instance != nullptr)
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

bool UZWInventoryManagerComponent::CanAddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	//@TODO: Add support for stack limit / uniqueness checks / etc...
	return true;
}

TArray<UZWInventoryItemInstance*> UZWInventoryManagerComponent::AddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount)
{
	TArray<UZWInventoryItemInstance*> AffectedInstances;
	
	if (!ItemDef || StackCount <= 0)
	{
		return AffectedInstances;
	}
	
	const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
	int32 RemainingCountToAdd = StackCount;
	
	if (Settings->bEnableStacking && Settings->StackCountTag.IsValid())
	{
		TArray<UZWInventoryItemInstance*> ExistingInstances = FindItemsByDefinition(ItemDef);

		for (UZWInventoryItemInstance* ExistingItem : ExistingInstances)
		{
			if (RemainingCountToAdd <= 0) break;

			const int32 CurrentStack = ExistingItem->GetStackCount();
			const int32 MaxStack = ExistingItem->GetMaxStackCount();

			if (CurrentStack < MaxStack)
			{
				const int32 RoomLeft = MaxStack - CurrentStack;
				const int32 AmountToAddHere = FMath::Min(RoomLeft, RemainingCountToAdd);

				ExistingItem->AddStatTagStack(Settings->StackCountTag, AmountToAddHere);
				
				RemainingCountToAdd -= AmountToAddHere;
				AffectedInstances.Add(ExistingItem);
			}
		}
	}
	
	// 2. Jeśli zostało nam jeszcze coś do dodania, tworzymy nowe sloty
	while (RemainingCountToAdd > 0)
	{
		int32 MaxStack = 1;
		
		// Opcjonalnie: odczytanie MaxStack przed utworzeniem nowej instancji z CDO (Default Object)
		if (Settings->bEnableStacking)
		{
			if (const UZWInventoryFragment_SetStats* StatFragment = ItemDef.LoadSynchronous()->FindFragmentByClass<UZWInventoryFragment_SetStats>())
			//if (const UZWInventoryFragment_SetStats* StatFragment = Cast<UZWInventoryFragment_SetStats>(ItemDef.LoadSynchronous()->FindFragmentByClass(UZWInventoryFragment_SetStats::StaticClass()));
			{
				MaxStack = StatFragment->GetItemStatByTag(Settings->MaxStackCountTag);
				MaxStack = MaxStack > 0 ? MaxStack : 1;
			}
		}

		int32 AmountForNewInstance = Settings->bEnableStacking ? FMath::Min(MaxStack, RemainingCountToAdd) : 1;

		// Tworzymy nową instancję korzystając z Twojej metody
		UZWInventoryItemInstance* NewInstance = InventoryList.AddEntry(ItemDef, AmountForNewInstance);
		
		if (!NewInstance) break;		

		if (Settings->bEnableStacking && Settings->StackCountTag.IsValid())
		{
			NewInstance->AddStatTagStack(Settings->StackCountTag, AmountForNewInstance);	
		}			

		if (IsUsingRegisteredSubObjectList() && IsReadyForReplication())
		{
			AddReplicatedSubObject(NewInstance);
		}

		RemainingCountToAdd -= AmountForNewInstance;
		AffectedInstances.Add(NewInstance);
	}

	return AffectedInstances;
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

TArray<UZWInventoryItemInstance*> UZWInventoryManagerComponent::FindItemsByDefinition(
	TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const
{
	TArray<UZWInventoryItemInstance*> FoundItems;
	
	for (const FZWInventoryEntry& Entry : InventoryList.Entries)
	{
		UZWInventoryItemInstance* Instance = Entry.Instance;

		if (IsValid(Instance) && Instance->GetItemDef() == ItemDef)
		{
			FoundItems.Add(Instance);
		}
	}

	return FoundItems;
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

		if (IsValid(Instance) && Instance->GetItemDef() == ItemDef)
		{
			// We now query the Instance itself for its tag-based stack count!
			TotalCount += Instance->GetStackCount(); 
		}
	}

	return TotalCount;
}

bool UZWInventoryManagerComponent::ConsumeItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 NumToConsume)
{
	AActor* OwningActor = GetOwner();
	if (!OwningActor || !OwningActor->HasAuthority() || NumToConsume <= 0)
	{
		return false;
	}

	int32 TotalAvailable = GetTotalItemCountByDefinition(ItemDef);
	if (TotalAvailable < NumToConsume)
	{
		return false; // Not enough items to consume
	}

	const UZWInventorySettings* Settings = GetDefault<UZWInventorySettings>();
	int32 RemainingToConsume = NumToConsume;
	
	TArray<UZWInventoryItemInstance*> ExistingInstances = FindItemsByDefinition(ItemDef);

	for (UZWInventoryItemInstance* Instance : ExistingInstances)
	{
		if (RemainingToConsume <= 0) break;

		const int32 CurrentStack = Instance->GetStackCount();
		const int32 AmountToConsumeHere = FMath::Min(CurrentStack, RemainingToConsume);

		if (Settings->bEnableStacking && Settings->StackCountTag.IsValid())
		{
			Instance->RemoveStatTagStack(Settings->StackCountTag, AmountToConsumeHere);
		}

		RemainingToConsume -= AmountToConsumeHere;

		// If the stack is now empty (or theoretically <= 0), destroy the instance entirely
		// Note: Ensure your GetStackCount() returns 0 or you handle this logic according to your Tag implementation
		if (Instance->GetStackCount() <= 0 || (!Settings->bEnableStacking))
		{
			RemoveItemInstance(Instance);
		}
	}

	return RemainingToConsume == 0;
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
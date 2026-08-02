// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWInventoryItemInstance.h"
#include "Components/ActorComponent.h"
#include "Net/Serialization/FastArraySerializer.h"
#include "ZWInventoryManagerComponent.generated.h"

class UZWInventoryItemDefinition;
class UZWInventoryItemInstance;
class UZWInventoryManagerComponent;
class UObject;
struct FFrame;
struct FZWInventoryList;
struct FReplicationFlags;

USTRUCT()
struct FZWInventoryEntrySaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	UClass* InstanceClass;

	UPROPERTY(SaveGame)
	UObject* Outer;
	
	UPROPERTY(SaveGame)
	FZWInventoryItemInstanceSaveData InventoryItemInstanceSaveData;

	UPROPERTY(SaveGame)
	TArray<uint8> Data;

	FZWInventoryEntrySaveData()
	{
		InstanceClass = nullptr;
		Outer = nullptr;
	}
	
};

USTRUCT()
struct FZWInventoryListSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	TArray<FZWInventoryEntrySaveData> EntryRecords;

	UPROPERTY(SaveGame)
	TArray<uint8> Data;
	
};

USTRUCT()
struct FZWInventoryManagerComponentSaveData
{
	GENERATED_BODY()

	UPROPERTY(SaveGame)
	FZWInventoryListSaveData InventoryListSaveData;

	UPROPERTY(SaveGame)
	TArray<TSoftObjectPtr<UZWInventoryItemDefinition>> ItemDefinitions;

	UPROPERTY(SaveGame)
	TArray<FZWInventoryItemDefinitionSaveData> DefinitionSaveDatas;

	UPROPERTY(SaveGame)
	TMap<TSoftObjectPtr<UZWInventoryItemDefinition>, FZWInventoryItemDefinitionSaveData> DefinitionRecords;
	
	UPROPERTY(SaveGame)
	TArray<uint8> Data;
	
};

/** A message when an item is added to the inventory */
USTRUCT(BlueprintType)
struct FZWInventoryChangeMessage
{
	GENERATED_BODY()

	//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?
	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	TObjectPtr<UActorComponent> InventoryOwner = nullptr;

	UPROPERTY(BlueprintReadOnly, Category = Inventory)
	TObjectPtr<UZWInventoryItemInstance> Instance = nullptr;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 NewCount = 0;

	UPROPERTY(BlueprintReadOnly, Category=Inventory)
	int32 Delta = 0;
};

/** A single entry in an inventory */
USTRUCT(BlueprintType)
struct FZWInventoryEntry : public FFastArraySerializerItem
{
	GENERATED_BODY()

	FZWInventoryEntry()
	{}

	FString GetDebugString() const;
	
	// --- CLIENT FUNCTIONS (FAST ARRAY) ---
	// These functions are called automatically on the Client after replication
	void PreReplicatedRemove(const FZWInventoryList& InArraySerializer);
	void PostReplicatedAdd(const FZWInventoryList& InArraySerializer);
	void PostReplicatedChange(const FZWInventoryList& InArraySerializer);

	// --- HELPER FUNCTION ---
	// "Mirror" function - updates the Tags in the instance based on the current StackCount
	void SyncInstanceToEntry();

	void SaveEntry(FZWInventoryEntrySaveData& EntrySaveData);
	void LoadEntry(FZWInventoryEntrySaveData& EntrySaveData);

private:
	friend FZWInventoryList;
	friend UZWInventoryManagerComponent;

	UPROPERTY(SaveGame)
	TObjectPtr<UZWInventoryItemInstance> Instance = nullptr;

	UPROPERTY()
	int32 StackCount = 0;

	UPROPERTY(NotReplicated)
	int32 LastObservedCount = INDEX_NONE;
};

/** List of inventory items */
USTRUCT(BlueprintType)
struct FZWInventoryList : public FFastArraySerializer
{
	GENERATED_BODY()

public:	
	FZWInventoryList()
		: OwnerComponent(nullptr)
	{
	}

	FZWInventoryList(UActorComponent* InOwnerComponent)
		: OwnerComponent(InOwnerComponent)
	{
	}

	TArray<UZWInventoryItemInstance*> GetAllItems() const;
	
	//~FFastArraySerializer contract
	void PreReplicatedRemove(const TArrayView<int32> RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize);
	//~End of FFastArraySerializer contract
	
	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
	{
		return FastArrayDeltaSerialize<FZWInventoryEntry, FZWInventoryList>(Entries, DeltaParms, *this);
	}
	
	UZWInventoryItemInstance* AddEntry(TSoftObjectPtr<UZWInventoryItemDefinition> ItemClass, int32 StackCount);
	void AddEntry(UZWInventoryItemInstance* Instance);

	void RemoveEntry(UZWInventoryItemInstance* Instance);

	void SaveInventoryList(FZWInventoryListSaveData& InventoryListSaveData);

	void LoadInventoryList(FZWInventoryListSaveData& InventoryListSaveData);

private:
	void BroadcastChangeMessage(FZWInventoryEntry& Entry, int32 OldCount, int32 NewCount);

	friend UZWInventoryManagerComponent;
	
	// Replicated list of items
	UPROPERTY(SaveGame)
	TArray<FZWInventoryEntry> Entries;

	UPROPERTY(NotReplicated)
	TObjectPtr<UActorComponent> OwnerComponent;
};

template<>
struct TStructOpsTypeTraits<FZWInventoryList> : public TStructOpsTypeTraitsBase2<FZWInventoryList>
{
	enum { WithNetDeltaSerializer = true };
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemAddedDelegate, FText, ItemName, int32, StackCount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZWINVENTORY_API UZWInventoryManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UZWInventoryManagerComponent(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());

	UFUNCTION(BlueprintCallable, /*BlueprintAuthorityOnly,*/ Category=Inventory)
	bool CanAddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, /*BlueprintAuthorityOnly,*/ Category=Inventory)
	TArray<UZWInventoryItemInstance*> AddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount = 1);

	UFUNCTION(BlueprintCallable, /*BlueprintAuthorityOnly,*/ Category=Inventory)
	void AddItemInstance(UZWInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, /*BlueprintAuthorityOnly,*/ Category=Inventory)
	void RemoveItemInstance(UZWInventoryItemInstance* ItemInstance);

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure=false)
	TArray<UZWInventoryItemInstance*> GetAllItems() const;

	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	TArray<UZWInventoryItemInstance*> FindItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)
	UZWInventoryItemInstance* FindFirstItemStackByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory, BlueprintPure)	
	int32 GetTotalItemCountByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef) const;
	
	UFUNCTION(BlueprintCallable, Category=Inventory)
	bool ConsumeItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 NumToConsume);

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	FZWInventoryManagerComponentSaveData SaveInventoryManager();

	void LoadInventoryManager(FZWInventoryManagerComponentSaveData& InventoryManagerSaveData);
	
	UPROPERTY(BlueprintAssignable, Category= "Inventory")
	FItemAddedDelegate OnItemAdded;

private:
	UPROPERTY(VisibleAnywhere, Category=Inventory, SaveGame)
	FZWInventoryList InventoryList;
	
	void SortItemStacks(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef);
};

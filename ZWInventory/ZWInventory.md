# ZWInventory — Technical Documentation

## 1. Overview

**ZWInventory** is a runtime inventory system plugin for Unreal Engine, part of the ZWSuite plugin family. It is derived from (and clearly modeled on) Epic's Lyra starter-game inventory architecture, adapted into the `ZW*` namespace and extended with:

- **Tag-driven stacking** — stack counts, max stack size, and total (lifetime) stack limits are expressed as *GameplayTags* stored as `FGameplayTagStack` counters on item instances, configured globally in Project Settings (`UZWInventorySettings`).
- **A replicated inventory manager** (`UZWInventoryManagerComponent`) using `FFastArraySerializer` delta replication, with client-side mirror functions (`SyncInstanceToEntry`) and entry-level replication callbacks.
- **A pickup interface** (`IPickupable`) plus a blueprint function library (`UPickupableStatics`) that funnels pickups (template-based or instance-based) into an inventory manager.
- **SaveGame support** throughout: definitions, fragments, instances, entries, lists and the manager itself all have `*SaveData` structs and Save/Load paths built on `FMemoryWriter`/`FMemoryReader` + `FObjectAndNameAsStringProxyArchive`.
- **A per-player registry subsystem** (`UZWInventorySubsystem`) mapping `APlayerState*` → `UZWInventoryManagerComponent*`.
- **Editor tooling** (module `ZWInventoryEditor`): an asset category, asset-type actions, and an asset factory for `UZWInventoryItemDefinition`.
- A **pickup component** (`UZWInventoryComponent`) that binds a `UZWInteractionComponent` (from the sibling plugin **ZWInteraction**) and, on interact, streams the configured pickup inventory into the player's `UZWInventoryManagerComponent` (located on the PlayerState) and destroys the owner actor.

Big-picture flow:
`ItemDefinition DataAsset` (with instanced `Fragments`) → pickup (IPickupable on actor/component) → `UPickupableStatics::AddPickupToInventory` → `UZWInventoryManagerComponent::AddItemDefinition/AddItemInstance` → stacked instances kept in the replicated `FZWInventoryList` (FastArray) → change broadcast via `FZWInventoryChangeMessage` (currently stubbed) and `OnItemAdded` delegate.

---

## 2. Metadata (.uplugin)

Source: `ZWInventory.uplugin`

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | `ZWInventory` |
| Description | *(empty)* |
| Category | `Other` |
| CreatedBy | `tiramisoo` |
| CanContainContent | true |
| EnabledByDefault | false |
| IsBetaVersion / IsExperimentalVersion | false / false |
| Installed | false |

**Modules** (both `LoadingPhase: Default`):

| Module | Type |
|---|---|
| `ZWInventory` | Runtime |
| `ZWInventoryEditor` | Editor |

**Plugin dependencies**: `ZWInteraction` (Enabled: true).

Resources: `Resources/Icon128.png`.

---

## 3. Sub-modules (Build.cs)

### 3.1 `ZWInventory.Build.cs` (runtime module — `Source/ZWInventory/`)

- `PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;`
- **PublicDependencyModuleNames**: `Core`, `GameplayTags`
- **PrivateDependencyModuleNames**: `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `ZWInteraction`, `NetCore`, `DeveloperSettings`
- PublicIncludePaths / PrivateIncludePaths / DynamicallyLoadedModuleNames: empty (template stubs).

Notes: `Slate`/`SlateCore` are linked but no Slate code exists in the runtime module (copy-paste from template). `NetCore` backs the `FFastArraySerializer` usage; `DeveloperSettings` backs `UZWInventorySettings`; `ZWInteraction` backs `UZWInventoryComponent`.

### 3.2 `ZWInventoryEditor.Build.cs` (editor module — `Source/ZWInventoryEditor/`)

- `PCHUsage = UseExplicitOrSharedPCHs`
- **PublicDependencyModuleNames**: `Core`, `AssetTools`
- **PrivateDependencyModuleNames**: `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `UnrealEd`, `ZWInventory`

---

## 4. Public API — classes

All engine-facing classes are prefixed `ZWInventory…`. Full signature listing follows. `brak` marks fields absent from the code.

### 4.1 Item definition & fragment system — `ZWInventoryItemDefinition.h`

#### `UZWInventoryItemDefinition : public UDataAsset`
`UCLASS(Blueprintable, Const, EditInlineNew)` — **ZWINVENTORY_API**. The canonical "what is an item" asset. `Const` is unusual (Lyra-style read-only definitions).

| Member | Kind | Signature / Notes |
|---|---|---|
| `DisplayName` | UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display) | `FText` — displayed item name; used in `OnItemAdded` broadcast. |
| `Fragments` | UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category=Display, **Instanced**) | `TArray<TObjectPtr<UZWInventoryItemFragment>>` — instanced, edit-inline fragment objects configured per asset. |
| ctor | method | `UZWInventoryItemDefinition(const FObjectInitializer& ObjectInitializer = FObjectInitializer::Get());` (empty body in .cpp). |
| `SaveDefinition` | method (not UFUNCTION) | `void SaveDefinition(FZWInventoryItemDefinitionSaveData&)` — calls `SaveFragment` on every fragment (record emplacement commented out), then byte-serializes the whole object: `FMemoryWriter(SaveData.Data, true)` + `FObjectAndNameAsStringProxyArchive(MemWriter, true)`, set `ArIsSaveGame = true`, and `Serialize(Ar`. |
| `LoadDefinition` | method | `void LoadDefinition(FZWInventoryItemDefinitionSaveData&)` — deserializes its own data, then for each `FZWInventoryItemFragmentSaveData` record: `NewObject<UZWInventoryItemFragment>(this, FragmentData.FragmentClass)`, calls `LoadFragment`, and appends the fresh fragment to `Fragments`. Logs each fragment struct name via `UE_LOG(LogTemp, Log, …)`. |
| `FindFragmentByClass` | method | `const UZWInventoryItemFragment* FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const` — linear scan of `Fragments` with `IsA` on each. |
| `FindFragmentByClass<T>` | C++ template overload | `template <typename FragmentClass> const FragmentClass* FindFragmentByClass() const` — the two calls together, with `Cast` result. |

#### `UZWInventoryItemFragment : public UObject` *(base class for all item fragments)*
`UCLASS(DefaultToInstanced, EditInlineNew, Abstract, Blueprintable)` — abstract, instanced inside definitions.

| Member | Kind | Signature / Notes |
|---|---|---|
| `OnItemInstanceCreated` | UPROPERTY(BlueprintAssignable, Category=ZWInventoryItemFragment, DisplayName="On Instance Created") | `FOnInstanceCreatedDelegate` |
| Delegate | typedef | `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInstanceCreatedDelegate, UZWInventoryItemInstance*, Instance);` |
| `OnInstanceCreated` | virtual | `virtual void OnInstanceCreated(UZWInventoryItemInstance* Instance) const` — default implementation just `OnItemInstanceCreated.Broadcast(Instance)`. |
| `SaveFragment` | virtual | `virtual void SaveFragment(FZWInventoryItemDefinitionSaveData&)` — empty default. |
| `LoadFragment` | virtual | `virtual void LoadFragment(FZWInventoryItemFragmentSaveData&)` — empty default. |

Code comment in-file: `//@TODO: Make it a blueprint event so the fragments can be created`.

#### Save-data structs (same header)

| Struct | UPROPERTY(SaveGame) fields |
|---|---|
| `FZWInventoryItemFragmentSaveData` (USTRUCT) | `UClass* FragmentClass` (ctor-null), `TArray<uint8> Data` |
| `FZWInventoryItemDefinitionSaveData` (USTRUCT) | `UClass* Class` (ctor-null), `TArray<FZWInventoryItemFragmentSaveData> FragmentRecords`, `TArray<uint8> Data` |

#### `UZWInventoryFunctionLibrary : public UBlueprintFunctionLibrary`
`UCLASS()` — plain library (no prefix concerns). In-file comment: *"Make into a subsystem instead?"*

| Member | Kind | Notes |
|---|---|---|
| `FindItemDefinitionFragment` | UFUNCTION(BlueprintCallable, meta=(DeterminesOutputType=FragmentClass)), static | `static const UZWInventoryItemFragment* FindItemDefinitionFragment(TSubclassOf<UZWInventoryItemDefinition> ItemDef, TSubclassOf<UZWInventoryItemFragment> FragmentClass)` — resolves the CDO of `ItemDef` (`GetDefault<…>`) and calls its `FindFragmentByClass`; returns `nullptr` for null inputs. |

---

### 4.2 Item instance — `ZWInventoryItemInstance.h`

#### `EHasFragment` (UENUM, uint8)
Values: `Valid`, `NotValid`. **Not referenced anywhere else in the module** (declared in the header but unused in this snapshot).

#### `FZWInventoryItemInstanceSaveData` (USTRUCT)
SaveGame fields: `FZWInventoryItemDefinitionSaveData InventoryItemDefinitionSaveData`, `TArray<uint8> Data`.

#### `UZWInventoryItemInstance : public UObject`
`UCLASS(BlueprintType)`. A concrete runtime item — defers static behavior to its `ItemDefinition`, holds dynamic state exclusively as gameplay-tag stacks.

| Member | Kind | Signature / Notes |
|---|---|---|
| `AddStatTagStack` | UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category=Inventory) | `void AddStatTagStack(FGameplayTag Tag, int32 StackCount)` — adds stacks to a tag (no-op if count < 1). |
| `RemoveStatTagStack` | UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly, Category= Inventory) | `void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount)` |
| `GetStatTagStackCount` | UFUNCTION(BlueprintCallable, Category=Inventory) | `int32 GetStatTagStackCount(FGameplayTag Tag) const` — 0 if absent. |
| `HasStatTag` | UFUNCTION(BlueprintCallable, Category=Inventory) | `bool HasStatTag(FGameplayTag Tag) const` |
| `GetStackCount` | UFUNCTION(BlueprintPure, Category="Inventory\|Stacking") | `int32 GetStackCount() const` — if stacking globally disabled or `StackCountTag` invalid → 1; else `FMath::Max(0, GetStatTagStackCount(StackCountTag))`. |
| `GetMaxStackCount` | UFUNCTION(BlueprintPure, Category="Inventory\|Stacking") | `int32 GetMaxStackCount() const` — static limit read from this instance's definition's `UZWInventoryFragment_SetStats` map under `MaxStackCountTag`; defaults to 1 when stacking off / tag invalid / fragment missing / value ≤ 0. |
| `GetTotalStackCount` | UFUNCTION(BlueprintPure, Category="Inventory\|Stacking") | `int32 GetTotalStackCount() const` — lifetime cap from `TotalStackCountTag` in the stats fragment; returns **-1** ("infinity / no limit") when stacking off / tag invalid / fragment missing. |
| `GetItemDef` | C++ inline | `TSoftObjectPtr<UZWInventoryItemDefinition> GetItemDef() const` |
| `GetItemDefinition` | UFUNCTION(BlueprintCallable, Category=Inventory) | `TSoftObjectPtr<UZWInventoryItemDefinition> GetItemDefinition()` — a second, non-const accessor of the same soft pointer. |
| `FindFragmentByClass` | UFUNCTION(BlueprintCallable, BlueprintPure=false, meta=(DeterminesOutputType=FragmentClass)) | `const UZWInventoryItemFragment* FindFragmentByClass(TSubclassOf<UZWInventoryItemFragment> FragmentClass) const` — `LoadSynchronous()` on the soft definition pointer, then delegates to definition lookup. Template overload `FindFragmentByClass<ResultClass>()` performs a raw C-style cast instead of `Cast` (unlike the definition-side template). |
| `SaveItemInstance` | method | `void SaveItemInstance(FZWInventoryItemInstanceSaveData&)` — builds a `FZWInventoryItemDefinitionSaveData` (`Class = ItemDef->GetClass()`), `LoadSynchronous()` the def, and if valid **returns early after `SaveDefinition`** — i.e. the assigment of `SaveData.InventoryItemDefinitionSaveData` and the instance's own `Serialize(Ar)` are skipped in the valid case (a suspected bug; see §8). |
| `LoadItemInstance` | method | `void LoadItemInstance(FZWInventoryItemInstanceSaveData&)` — reads own bytes, then constructs a *new transient* `UZWInventoryItemDefinition` (`NewObject<UZWInventoryItemDefinition>()`) and `LoadDefinition` from the stored record. It is **not assigned to `ItemDef`**, so the loaded definition leaks as a fresh scratch object. |
| `SetItemDef` | C++ (private) | `void SetItemDef(TSoftObjectPtr<UZWInventoryItemDefinition> InDef)` — private, `friend struct FZWInventoryList` is the only writer. |

Private state:

| Field | UPROPERTY | Meaning |
|---|---|---|
| `StatTags` | `FGameplayTagStackContainer` | Heading "stacking": all per-instance counters live here as tag stacks. |
| `ItemDef` | `TSoftObjectPtr<UZWInventoryItemDefinition>` | Soft pointer to owning definition. |

Commented-out (in .cpp) includes for `Net/UnrealNetwork.h` and UE Iris `ReplicationFragmentUtil.h`; the class is intended as a replicated sub-object (`AddReplicatedSubObject` from the manager) but has no replicated UPROPERTYs beyond the merely-touched headers.

---

### 4.3 Gameplay tag stacks — `GameplayTagStack.h` / `.cpp` (Epic-derived)

#### `FGameplayTagStack` (USTRUCT, BlueprintType)
One tag + count. Constructors: default and `(FGameplayTag InTag, int32 InStackCount)`. `FString GetDebugString() const` → `"%sx%d"`. Private `UPROPERTY() FGameplayTag Tag;` `int32 StackCount = 0;` `friend FGameplayTagStackContainer`.

#### `FGameplayTagStackContainer` (USTRUCT, BlueprintType)
Container used as item instance stat storage. Public API:

| Member | Signature |
|---|---|
| `AddStack` | `void AddStack(FGameplayTag Tag, int32 StackCount)` |
| `RemoveStack` | `void RemoveStack(FGameplayTag Tag, int32 StackCount)` |
| `GetStackCount` | inline `int32 GetStackCount(FGameplayTag Tag) const` → `TagToCountMap.FindRef(Tag)` |
| `ContainsTag` | inline `bool ContainsTag(FGameplayTag Tag) const` |

Storage: `TArray<FGameplayTagStack> Stacks` (UPROPERTY, commented "replicated") + a non-UPROPERTY acceleration map `TMap<FGameplayTag,int32> TagToCountMap`.

Implementation details:
- `AddStack`: guards invalid tag with `FFrame::KismetExecutionMessage(..., Warning)`; if the tag exists, updates both the array element and map; otherwise `Stacks.Emplace_GetRef(Tag, StackCount)` + map insert.
- `RemoveStack`: invalid-tag guard; iterating `CreateIterator()`, if count ≤ stored count the entry is removed entirely (`It.RemoveCurrent()` + map remove), else decremented. `//@TODO: Should we error if you try to remove a stack that doesn't exist…?`
- Legacy `FFastArraySerializer` machinery (`PreReplicatedRemove`/`PostReplicatedAdd`/`PostReplicatedChange`, `NetDeltaSerialize`, `WithNetDeltaSerializer` trait, `MarkItemDirty` calls) is present but **commented out** — the container is currently non-replicated and non-dirtyable.

---

### 4.4 Manager component — `ZWInventoryManagerComponent.h`

FastArray-based, replicated-by-default inventory list on any owning actor (typically `PlayerState`).

#### Save-data structs (all USTRUCT, all fields `UPROPERTY(SaveGame)`)

| Struct | Fields |
|---|---|
| `FZWInventoryEntrySaveData` | `UClass* InstanceClass` (ctor-null), `UObject* Outer` (ctor-null), `FZWInventoryItemInstanceSaveData InventoryItemInstanceSaveData`, `TArray<uint8> Data` |
| `FZWInventoryListSaveData` | `TArray<FZWInventoryEntrySaveData> EntryRecords`, `TArray<uint8> Data` |
| `FZWInventoryManagerComponentSaveData` | `FZWInventoryListSaveData InventoryListSaveData`, `TArray<TSoftObjectPtr<UZWInventoryItemDefinition>> ItemDefinitions`, `TArray<FZWInventoryItemDefinitionSaveData> DefinitionSaveDatas`, `TMap<TSoftObjectPtr<UZWInventoryItemDefinition>, FZWInventoryItemDefinitionSaveData> DefinitionRecords`, `TArray<uint8> Data` |

#### `FZWInventoryChangeMessage` (USTRUCT, BlueprintType)
IPC payload for inventory changes (designed for the (commented-out) GameplayMessageSubsystem):

| Field | UPROPERTY | Type |
|---|---|---|
| `InventoryOwner` | BlueprintReadOnly, Category=Inventory | `TObjectPtr<UActorComponent>` (null) |
| `Instance` | BlueprintReadOnly, Category=Inventory | `TObjectPtr<UZWInventoryItemInstance>` (null) |
| `NewCount` | BlueprintReadOnly, Category=Inventory | `int32` (0) |
| `Delta` | BlueprintReadOnly, Category=Inventory | `int32` (0) |

In-file: `//@TODO: Tag based names+owning actors for inventories instead of directly exposing the component?`

#### `FZWInventoryEntry : public FFastArraySerializerItem` (USTRUCT, BlueprintType)
One inventory slot.

| Member | Kind | Notes |
|---|---|---|
| `GetDebugString` | method | `"%s (%d x %s)"` — instance name, StackCount, definition name (`LoadSynchronous`); empty string on invalid def. |
| `PreReplicatedRemove(const FZWInventoryList&)` | per-entry FastArray callback (client side) | Body: `SyncInstanceToEntry` style messages all commented; only no-op remains. |
| `PostReplicatedAdd(const FZWInventoryList&)` | per-entry callback | Calls `SyncInstanceToEntry()`; message broadcast commented; sets `LastObservedCount = StackCount`. |
| `PostReplicatedChange(const FZWInventoryList&)` | per-entry callback | Same double sync + bookkeeping pattern. |
| `SyncInstanceToEntry` | method | "Mirror" function: forces the instance's `StackCountTag` stat to the entry's `StackCount` — removes the whole current tag count and re-adds the structural value, guaranteeing 100% consistency between the replicated struct and the instance. |
| `SaveEntry` | method | `void SaveEntry(FZWInventoryEntrySaveData&)` — serializes instance into save data, stores `InstanceClass` and `Outer`. In-source warning: `//@TODO: For some reason Item Instance doesnt get saved/loaded correctly and results with a crash`. |
| `LoadEntry` | method | `void LoadEntry(FZWInventoryEntrySaveData&)` — `NewObject<UZWInventoryItemInstance>(Outer, InstanceClass->StaticClass())`, `LoadItemInstance`, assign. |
| `Instance` | UPROPERTY(SaveGame), private | `TObjectPtr<UZWInventoryItemInstance>` |
| `StackCount` | UPROPERTY (replicated), private | `int32` (0) |
| `LastObservedCount` | UPROPERTY(NotReplicated), private | `int32 = INDEX_NONE` |

Friends: `FZWInventoryList`, `UZWInventoryManagerComponent`.

#### `FZWInventoryList : public FFastArraySerializer` (USTRUCT, BlueprintType)
Contructors: `FZWInventoryList()` (OwnerComponent null) and `FZWInventoryList(UActorComponent* InOwnerComponent)`. `TStructOpsTypeTraits` declares `WithNetDeltaSerializer = true`.

| Member | Kind | Notes |
|---|---|---|
| `GetAllItems` | method | `TArray<UZWInventoryItemInstance*> GetAllItems() const` — valid non-null instances only. |
| `PreReplicatedRemove / PostReplicatedAdd / PostReplicatedChange` | FastArray contract, `(const TArrayView<int32>, int32)` | Each broadcasts a change message (`(Stack.StackCount → 0)`, `(0 → StackCount)`, `(LastObservedCount → StackCount)`) and updates `LastObservedCount`. Change callback `check()`s stat was synced. |
| `NetDeltaSerialize` | inline | `FastArrayDeltaSerialize<FZWInventoryEntry, FZWInventoryList>(Entries, DeltaParms, *this)`. |
| `AddEntry` (definition form) | method | `UZWInventoryItemInstance* AddEntry(TSoftObjectPtr<UZWInventoryItemDefinition> ItemClass, int32 StackCount)` — `check` on def/owner; `check(OwningActor->HasAuthority())`; appends entry with `NewObject<UZWInventoryItemInstance>(OwningActor)` — outer is the **actor** (component originally, reverted due to `UE-127172`); `SetItemDef`; calls every non-null fragment's `OnInstanceCreated(Instance)`; sets `StackCount`; `MarkItemDirty(NewEntry)`; commented-out multiplayer sync call. |
| `AddEntry` (instance form) | method overload | `void AddEntry(UZWInventoryItemInstance* Instance)` — **`unimplemented()`** (dead path; `AddItemInstance` on the manager calls the unimplemented overload through `InventoryList.AddEntry(ItemInstance)`). |
| `RemoveEntry` | method | `void RemoveEntry(UZWInventoryItemInstance*)` — removes matching entries by pointer identity, `MarkArrayDirty()`. |
| `SaveInventoryList` | method | Iterates entries → `Entry.SaveEntry` → `EntryRecords`. Note: copies each `FZWInventoryEntry` **by value** at call site (`for (FZWInventoryEntry Entry : Entries)` in .cpp). |
| `LoadInventoryList` | method | For each record: log line (dereferences `Entry.InstanceClass->StaticClass()` then `GetName()`), `NewEntry.LoadEntry`, append. |
| `BroadcastChangeMessage` | method (private) | Builds `FZWInventoryChangeMessage{OwnerComponent, Entry.Instance, NewCount, NewCount - OldCount}`. Actual broadcast through `UGameplayMessageSubsystem` and its static tag `Inventory.Message.StackChanged` are **commented out**, so the message struct is currently built and dropped. |
| `Entries` | UPROPERTY(SaveGame), private | `TArray<FZWInventoryEntry>` — the replicated array. |
| `OwnerComponent` | UPROPERTY(NotReplicated), private | `TObjectPtr<UActorComponent>` |

#### `UZWInventoryManagerComponent : public UActorComponent`
`UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))`. Replication: `SetIsReplicatedByDefault(true)` (ctor), designed for registered-replicated-sub-objects usage (instance is subobject; the *list* is the replicated property). Notably the stack-related UFUNCTIONs lose `BlueprintAuthorityOnly` (commented out).

**BlueprintCallable API:**

| Member | Signature | Behavior |
|---|---|---|
| `OnItemAdded` | UPROPERTY(BlueprintAssignable, Category="Inventory") `FItemAddedDelegate` — `DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FItemAddedDelegate, FText, ItemName, int32, StackCount)` | Fired on every successful `AddItemDefinition` (args: `ItemDef->DisplayName`, raw incoming `StackCount`). |
| `CanAddItemDefinition` | `bool CanAddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount = 1)` | Stub returning `true` (`//@TODO: Add support for stack limit / uniqueness checks / etc...`). |
| `AddItemDefinition` | `TArray<UZWInventoryItemInstance*> AddItemDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 StackCount = 1)` | The stacking engine. Rejects unloaded def / `StackCount <= 0` (empty result). Stage 1: fill *existing* stacks — for each `FindItemsByDefinition`, if `GetStackCount() < GetMaxStackCount()` add `min(RoomLeft, Remaining)` via `AddStatTagStack`. Stage 2: spawn new slots — while `RemainingCountToAdd > 0`, read MaxStack from the def's SetStats fragment (CDO via `LoadSynchronous`), `AmountForNewInstance = min(MaxStack, Remaining)` (1 when stacking off), `InventoryList.AddEntry(ItemDef, Amount)` + `AddStatTagStack`; registers `AddReplicatedSubObject(NewInstance)` **only when `IsUsingRegisteredSubObjectList() && IsReadyForReplication()`**. Ends with `OnItemAdded.Broadcast`. |
| `AddItemInstance` | `void AddItemInstance(UZWInventoryItemInstance* ItemInstance)` | Whole-instance add: `InventoryList.AddEntry(ItemInstance)` (which is `unimplemented()`), plus sub-object registration if applicable. |
| `RemoveItemInstance` | `void RemoveItemInstance(UZWInventoryItemInstance*)` | Removes from list + `RemoveReplicatedSubObject` when using registered sub-object list. |
| `GetAllItems` | `TArray<UZWInventoryItemInstance*> GetAllItems() const` |
| `FindItemsByDefinition` | `TArray<UZWInventoryItemInstance*> FindItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition>) const` — linear scan, pointer-equality on the soft def. |
| `FindFirstItemStackByDefinition` | `UZWInventoryItemInstance* …(ItemDef) const` |
| `GetTotalItemCountByDefinition` | `int32 GetTotalItemCountByDefinition(ItemDef) const` — sums `Instance->GetStackCount()` over all matching entries. |
| `ConsumeItemsByDefinition` | `bool ConsumeItemsByDefinition(TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef, int32 NumToConsume)` | Authority-only in practice: requires `GetOwner()->HasAuthority()`. Computes available count, fails if insufficient; drains from existing instances via `RemoveStatTagStack`, removes emptied instances (`GetStackCount() <= 0`, or always when stacking disabled); ends with `SortItemStacks`. Returns `RemainingToConsume == 0`. |
| `BeginPlay` | virtual | Registers self with `UZWInventorySubsystem` (via GameInstance). |
| `EndPlay` | virtual | Unregisters. |
| `SaveInventoryManager` | `FZWInventoryManagerComponentSaveData SaveInventoryManager()` — for each instance: `//@TODO We need to save also fragments`; calls `SaveDefinition` on the def (note `.SaveDefinition` invoked through `ItemDef` TSoftObjectPtr operator->), populates `ItemDefinitions` + `DefinitionRecords`; `InventoryList.SaveInventoryList` call is commented out. Serializes self into `Data` with `FMemoryReader MemWriter(...)` (naming quirk — reads into a writer variable). |
| `LoadInventoryManager` | `void LoadInventoryManager(FZWInventoryManagerComponentSaveData&)` — deserializes self, then for each `ItemDefinitions` entry: `LoadDefinition(…DefinitionRecords[ItemDef])` and `AddItemDefinition(ItemDef)` (stack default 1). |
| `InventoryList` | UPROPERTY(VisibleAnywhere, Category=Inventory, SaveGame), private | `FZWInventoryList` |
| `SortItemStacks` | private `void SortItemStacks(TSoftObjectPtr<UZWInventoryItemDefinition>)` | Authority-only compaction: finds the first non-full stack, fills it from subsequent stacks (removing them or advancing), `ensure(stack <= max)`. Warns via `LogTemp` when stacking disabled/invalid tag. |

---

### 4.5 Pickup interface — `IPickupable.h`

#### `FPickupTemplate` / `FPickupInstance` / `FInventoryPickup` (USTRUCT, BlueprintType)
- `FPickupTemplate`: `int32 StackCount = 1` (EditAnywhere), `TSoftObjectPtr<UZWInventoryItemDefinition> ItemDef` (EditAnywhere, BlueprintReadOnly) — *definition-based* stacking submission.
- `FPickupInstance`: `TObjectPtr<UZWInventoryItemInstance> Item = nullptr` — *pre-built instance* submission.
- `FInventoryPickup`: `TArray<FPickupInstance> Instances`, `TArray<FPickupTemplate> Templates`.

#### `UPickupable` / `IPickupable`
- `UINTERFACE(MinimalAPI, BlueprintType, meta=(CannotImplementInterfaceInBlueprint))` — native-only interface.
- `IPickupable` (ZWINVENTORY_API): single pure virtual `UFUNCTION(BlueprintCallable) virtual FInventoryPickup GetPickupInventory() const = 0;`

#### `UPickupableStatics : public UBlueprintFunctionLibrary`
| Member | Signature | Notes |
|---|---|---|
| ctor | `UPickupableStatics()` — empty, forwards `FObjectInitializer::Get()`. |
| `GetFirstPickupableFromActor` | UFUNCTION(BlueprintPure) `static TScriptInterface<IPickupable> GetFirstPickupableFromActor(AActor* Actor)` — tries actor-as-implementer, then `GetComponentsByInterface(UPickupable::StaticClass())`, returning the **first** component (explicitly noted: more sophisticated pickup distinction must be solved elsewhere). |
| `AddPickupToInventory` | UFUNCTION(BlueprintCallable, /*BlueprintAuthorityOnly,*/ meta=(WorldContext="Ability")) `static void AddPickupToInventory(UZWInventoryManagerComponent* InventoryComponent, TScriptInterface<IPickupable> Pickup)` — for each `Templates`: `AddItemDefinition(ItemDef, StackCount)`; for each `Instances`: `AddItemInstance(Item)`. |

---

### 4.6 Pickup component — `ZWInventoryComponent.h`

#### `UZWInventoryComponent : public UActorComponent, public IPickupable`
`UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))` — a world pickup's "give my stuff to the player" worker; owns / borrows a `UZWInteractionComponent` (from **ZWInteraction**).

| Member | Kind | Signature / Notes |
|---|---|---|
| ctor | method | Disables tick (`bCanEverTick = false`); lazily `CreateDefaultSubobject<UZWInteractionComponent>(TEXT("InteractionComponent"))` with `bAutoActivate = bIsInteractableFromStart`. |
| `GetPickupInventory` | IPickupable override + UFUNCTION-in-interface | Returns `StaticInventory`. |
| `SetPickupInventory` | method | `void SetPickupInventory(const FInventoryPickup& InPickupInventory)` |
| `ToggleInteraction` | UFUNCTION(BlueprintCallable, Category="InteractionSystem") | `void ToggleInteraction(bool bIsInteractable)` — only when `bIsInteractableFromStart == false`; activates/deactivates the interaction component. |
| `BeginPlay` | virtual | Re-finds the interaction component on the owner (`FindComponentByClass`), binds `OnInteract` → `SetupInteraction` (guarded by `IsAlreadyBound`); deactivates it unless `bIsInteractableFromStart`. |
| `OnComponentDestroyed` | virtual | Unbinds `SetupInteraction`. |
| `SetupInteraction` | UFUNCTION | Finds the `UZWInventoryManagerComponent` on the **PlayerState of the player character (index 0)** (commented alternative: character itself), calls `UPickupableStatics::AddPickupToInventory(Manager, this)`, then `GetOwner()->Destroy()`. Commented-out `NotifyFlowGraph()` hook. |

Protected/private properties:

| Field | UPROPERTY | Type / default |
|---|---|---|
| `StaticInventory` | EditAnywhere | `FInventoryPickup` |
| `bIsInteractableFromStart` | EditAnywhere, BlueprintReadWrite, Category="InteractionSystem" | `bool = true` |
| `InteractionComponent` | private (raw pointer, not UPROPERTY) | `UZWInteractionComponent*` |

---

### 4.7 Settings — `ZWInventorySettings.h`

#### `UZWInventorySettings : public UDeveloperSettings`
`UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Inventory Settings"))` — visible in **Project Settings → Plugins → ZWInventory** (category name `"ZW"`, section `"ZW Inventory Settings"` via `GetCategoryName()`/`GetSectionText()` overrides; guarded `WITH_EDITORONLY_DATA`). Ctor sets `bEnableStacking = false` (mirroring "default off" in the .cpp, `ZWInventorySettings.cpp`).

| Member | Signature | Meaning |
|---|---|---|
| `bEnableStacking` | `UPROPERTY(Config, EditAnywhere, Category="Inventory", DisplayName="Enable Stacking") bool` | Master switch for tag-based stacking. |
| `StackCountTag` | Config, EditAnywhere; `meta=(EditCondition="bEnableStacking", EditConditionHides)` | Gameplay tag recording *the quantity currently held* in an inventory stack of an item (explicitly "NOT the amount of items in a pickupable stack"). |
| `MaxStackCountTag` | same meta | Tag mapped, inside an item's SetStats fragment, to the per-stack cap; over-capacity spills into additional stacks. |
| `TotalStackCountTag` | same meta | Tag mapped in the fragment to the *overall possession limit*; when reached, only a partial pickup is taken and the rest stays on the level instance. |

---

### 4.8 Subsystem — `ZWInventorySubsystem.h`

#### `UZWInventorySubsystem : public UGameInstanceSubsystem`
(Header comment after untranslated Polish): "Subsystem for managing and accessing all inventory components in the game."

| Member | Signature | Notes |
|---|---|---|
| `RegisterInventory` | `void RegisterInventory(UZWInventoryManagerComponent* InventoryComponent)` — maps owner PlayerState → component (only if owner is `APlayerState`). |
| `UnregisterInventory` | `void UnregisterInventory(UZWInventoryManagerComponent*)` — removes the map entry for the owner PlayerState. |
| `GetInventoryForPlayer` | `UZWInventoryManagerComponent* GetInventoryForPlayer(const APlayerState* PlayerState) const` — weak-pointer lookup. |
| `PlayerInventories` | private `TMap<TWeakObjectPtr<const APlayerState>, TWeakObjectPtr<UZWInventoryManagerComponent>>` |

---

### 4.9 Runtime-level module header — `ZWInventory.h` / `.cpp`

`FZWInventoryModule : public IModuleInterface` with boilerplate `StartupModule`/`ShutdownModule` and `IMPLEMENT_MODULE(FZWInventoryModule, ZWInventory)`. No `zw*` logging category is defined anywhere in the module (logging goes through `LogTemp`).

### 4.10 Editor module — `ZWInventoryEditor/Public`, `.uplugin`-declared

#### `FZWInventoryEditorModule : public IModuleInterface`
`StartupModule()` loads `FAssetToolsModule`, registers a shared asset category `RegisterAdvancedAssetCategory(FName("Inventory"), "Inventory")` and registers `ZWInventoryItemDefinitionAssetActions`. Exposes `EAssetTypeCategories::Type GetInventoryAssetCategory() const`. `ShutdownModule()` is empty (category not unregistered — normal for advanced categories).

#### `ZWInventoryItemDefinitionAssetActions : public FAssetTypeActions_Base`
`ZWINVENTORYEDITOR_API`. Overrides: `GetName()` ("Inventory Item Definition"), `GetSupportedClass()` → `UZWInventoryItemDefinition::StaticClass()`, `GetTypeColor()` → `FColor::Cyan`, `GetCategories()` → the module's `Inventory_AssetCategory` (loads own module lazily).

#### `UZWInventoryItemDefinitionFactory : public UFactory`
`UCLASS()`. Ctor sets `SupportedClass = UZWInventoryItemDefinition::StaticClass()`, `bCreateNew = true`, `bEditAfterNew = true`. `FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)` → `NewObject<UZWInventoryItemDefinition>(InParent, InClass, InName, Flags | RF_Transactional)`.

---

## 5. Implementation (Private)

### 5.1 `ZWInventory.cpp`
`FZWInventoryModule::StartupModule()` / `ShutdownModule()` — empty stubs with the template comments; `LOCTEXT_NAMESPACE "FZWInventoryModule"`; `IMPLEMENT_MODULE`.

### 5.2 `ZWInventoryItemDefinition.cpp`
- `SaveDefinition` iterates fragments calling `SaveFragment(DefinitionSaveData)` — but the line that would emplace the built `FZWInventoryItemFragmentSaveData ItemFragmentSaveData` record into `FragmentRecords` is commented out, so per-fragment records never accumulate from this path; then serializes itself.
- `LoadDefinition` reads bytes, then recreates fragments from `FragmentRecords` (needs the records populated elsewhere) — each loaded fragment is instantly orphaned from serialization but re-appended to `Fragments`; verbose debug log of fragment struct names; contains commented-out fragment-class resolution attempt (`auto temp = FragmentData.FragmentClass->StaticClass()`).
- `FindFragmentByClass(TSubclassOf<…>)`: linear `IsA` scan.
- `UZWInventoryFunctionLibrary::FindItemDefinitionFragment`: CDO-based lookup.

### 5.3 `ZWInventoryItemInstance.cpp`
- Thin delegation to `StatTags` for add/remove/count/has.
- `GetStackCount` / `GetMaxStackCount` / `GetTotalStackCount` — the tag-based stacking contract (see §4.2). Max/Total both resolve against `UZWInventoryFragment_SetStats` on the loaded definition; Max coerces non-positive values to 1; Total uses -1 for "unlimited".
- `SaveItemInstance` — early-return ordering bug: when the definition loads, it returns *before* `SaveData.InventoryItemDefinitionSaveData = …` and before the instance's own `Serialize(Ar)`.
- `LoadItemInstance` — reconstitutes an isolated `UZWInventoryItemDefinition` that never lands in `ItemDef`.
- `SetItemDef` (private, friend-writable).
- `FindFragmentByClass` — `LoadSynchronous()` + delegate; note the odd post-`return` brace pattern (`if (!IsValid) return nullptr; { return …; }`).

### 5.4 `GameplayTagStack.cpp`
- `FGameplayTagStack::GetDebugString()` → `"%sx%d"`.
- `AddStack` / `RemoveStack` semantics per §4.3; all dirty/replication notifications commented out (`//MarkItemDirty(Stack)`, `//MarkArrayDirty()`), and the whole replication callback block is wrapped in `/* … */`.

### 5.5 `IPickupable.cpp`
- Empty ctor; `GetFirstPickupableFromActor` resolution order (actor → first component with the interface → null); `AddPickupToInventory` fan-out to the manager's template/instance adders.

### 5.6 `ZWInventoryComponent.cpp` (world pickup flow)
- Defaults (`InteractionComponent` default subobject), BeginPlay bind/unbind lifecycle, toggle semantics, and `SetupInteraction` hard-coding player index 0's PlayerState as the inventory host, consuming itself after depositing.

### 5.7 `ZWInventoryManagerComponent.cpp` — the core
- `FZWInventoryEntry`: debug string; per-entry client callbacks mostly commented (the component's *list-level* callbacks are alive and own broadcasting); `SyncInstanceToEntry` = full reset of the `StackCountTag` stat to the entry value; `SaveEntry`/`LoadEntry` with the in-source "doesnt get saved/loaded correctly and results with a crash" TODO; `LoadEntry` computes `EntrySaveData.InstanceClass->StaticClass()` (redundant `->StaticClass()` on an already-`UClass*`).
- `FZWInventoryList`: FastArray contract implemented; `BroadcastChangeMessage` builds the struct but posting to `UGameplayMessageSubsystem` with tag `Inventory.Message.StackChanged` is commented out (the static tag define is also commented out at file top).
- `FZWInventoryList::AddEntry(definition form)` — authority-checked instance creation, outer = actor, fragment `OnInstanceCreated` fan-out, `MarkItemDirty`.
- `FZWInventoryList::AddEntry(instance form)` — `unimplemented()`.
- `SaveInventoryList` closes over copies; `LoadInventoryList` logs every entry class name.
- `UZWInventoryManagerComponent` — replication defaults, subsystem register/unregister, the full stacking algorithm described in §4.4, `ConsumeItemsByDefinition` (drain → remove empty → sort), `SortItemStacks` (greedy fill from the first non-full stack), save/load paths with fragments TODO and the commented list save.

### 5.8 Editor — `Source/ZWInventoryEditor/Private`
- **`ZWInventoryEditor.cpp`**: AssetTools bootstrapping described in §4.10. This is the whole editor integration — i.e. this section *is* the editor factory / asset actions implementation, no custom detail panel, thumbnail, or asset actions like "Open editor" were added.
- **`ZWInventoryItemDefinitionAssetActions.cpp`**: name/color/categories getters only; no `HasActions`/`GetActions` overrides.
- **`ZWInventoryItemDefinitionFactory.cpp`**: creates a plain new `UZWInventoryItemDefinition` (transactional flag) with no seed content (empty fragments, empty display name).

### 5.9 `ZWInventorySettings.cpp`
Only the ctor sets `bEnableStacking = false`.

### 5.10 `ZWInventorySubsystem.cpp`
`RegisterInventory` / `UnregisterInventory` / `GetInventoryForPlayer` as per §4.8 — check-then-map with weak pointers; both registration returns rely on owner being PlayerState (world pickups never register).

### 5.11 `Fragments/ZWInventoryFragment_SetStats.cpp`
- `OnInstanceCreated(UZWInventoryItemInstance*) const override` — pushes every pair of `InitialItemStats` into the new instance via `AddStatTagStack`.
- `GetItemStatByTag(FGameplayTag) const` — map find, 0 default.

---

## 6. Configuration (.ini)

`Config/DefaultZWInventory.ini` — contains **only** `[CoreRedirects]`, no other settings (stacking defaults live in `UZWInventorySettings`, config=Game; the ini does not preseed them besides the redirects):

Class redirects (old → new):
- `InventoryItemDefinitionFactory` → `/Script/ZWInventoryEditor.ZWInventoryItemDefinitionFactory` (declared twice)
- `InventorySubsystem` → `/Script/ZWInventory.ZWInventorySubsystem` (declared twice)
- `InventoryComponent` → `/Script/ZWInventory.ZWInventoryComponent`
- `InventoryItemDefinition` → `/Script/ZWInventory.ZWInventoryItemDefinition` (twice)
- `InventoryManagerComponent` → `/Script/ZWInventory.ZWInventoryManagerComponent`
- `InventoryItemFragment` → `/Script/ZWInventory.ZWInventoryItemFragment`
- `InventoryFunctionLibrary` → `/Script/ZWInventory.ZWInventoryFunctionLibrary`
- `InventoryItemInstance` → `/Script/ZWInventory.ZWInventoryItemInstance`

Struct redirects:
- `InventoryItemFragmentSaveData` → `ZWInventoryItemFragmentSaveData`
- `InventoryItemDefinitionSaveData` → `ZWInventoryItemDefinitionSaveData`
- `InventoryEntrySaveData` → `ZWInventoryEntrySaveData`
- `InventoryListSaveData` → `ZWInventoryListSaveData`
- `InventoryManagerComponentSaveData` → `ZWInventoryManagerComponentSaveData`
- `InventoryChangeMessage` → `ZWInventoryChangeMessage`
- `InventoryEntry` → `ZWInventoryEntry`
- `InventoryList` → `ZWInventoryList`
- `InventoryItemInstanceSaveData` → `ZWInventoryItemInstanceSaveData`

These are compatibility shims from the pre-rename `Inventory*` names (Lyra lineage) to the current `ZW*` names, serialized into save data / bluperprints created under the old identifiers.

---

## 7. Dependencies inside ZWSuite

- **ZWInteraction** (plugin dependency, declared active in the .uplugin):
  - `UZWInventoryComponent` includes `ZWInteractionComponent.h` and actively creates/finds/binds `UZWInteractionComponent` (`OnInteract` delegate drives `SetupInteraction`); Build.cs private dep `ZWInteraction`.
- **ZWInventoryEditor** ← **ZWInventory** (Core/Engine private dep on module `ZWInventory` in the editor Build.cs; it references `UZWInventoryItemDefinition` and loads back into the runtime).
- Conversely, `ZWInventory` never calls editor code. No other ZWSuite modules are referenced; the ini/`Plugins` block touches only `ZWInteraction`.
- Function removals/separations: gameplay broadcasting (`UGameplayMessageSubsystem`) — Lyra heritage replaced by an in-component `OnItemAdded` BlueprintAssignable delegate; message struct remains for future use.

---

## 8. Notes / risks

1. **`SaveItemInstance` early return (skips storing its own data)** — instance-level save assignment and self-serialization occur only when the *def fails* to load; the "valid def" case short-circuits before touching `SaveData`. Combined with definition-side fragment-record assembly being commented out, save/load round-tripping of instances is degraded. In-source comment confirms known breakage: *"Item Instance doesnt get saved/loaded correctly and results with a crash"* on `SaveEntry`, plus `LoadInventoryList` dereferencing `InstanceClass->StaticClass()->GetName()` (calling `->StaticClass()` on a `UClass*` is redundant/unsafe-looking) — an entry list load can crash.
2. **`LoadItemInstance` builds a definition that is never attached to the instance** (fresh `NewObject<UZWInventoryItemDefinition>()` neither hits `ItemDef` nor any registry) — load restores bytes but leaves the definition disconnected.
3. `LoadItemInstance` logs `FZWInventoryItemFragmentSaveData.StaticStruct()->GetName()`; `FragmentClass` is used to `NewObject<UZWInventoryItemFragment>`; missing fragment class = crash risk. Same for `LoadDefinition` iterating `FragmentRecords` — only populated when the commented-out record-emplacement is restored.
4. `FZWInventoryList::AddEntry(instance)` is `unimplemented()` while the manager calls it (`AddItemInstance`); requires FastArray support; ready works only for pre-`BeginPlay` replicated sub-object setup.
5. Messages: `FZWInventoryChangeMessage` and `BroadcastChangeMessage` produce a struct nobody receives (broadcast commented out along with the gameplay tag); UI can only rely on `OnItemAdded` (fired on `Add`, *not* on remove/consume).
6. `CanAddItemDefinition` unconditionally returns `true`; stack limits/uniqueness are nominally reported in the settings (per def `TotalStackCountTag`) but not enforced in `AddItemDefinition`.
7. `GetTotalStackCount()` returning `-1` as "infinite" is *not* enforced anywhere when adding; there's no code today clipping a pick-up when `TotalStackCount` would be overreached despite the settings comment describing negative behavior.
8. Stacking semantics tightly coupled to `UZWInventorySettings`; setting up the three tags is a project-wide contract (`bEnableStacking=false` collapses everything to 1).
9. `SortItemStacks` guarantees full stacks and `ensure(current ≤ max)`; warning text still fires when *either* condition holds (condition is an OR with `!bEnableStacking` mid-warning, minor mismatch).
10. `StaticInventory` and the manager's `InventoryList` both rely on FastArray/reflection consistency; DOREPREC is commented out.
11. Polish source comments (manager; subsystem header) — refactor/original-language leftovers.
12. Missing pieces: no UFUNCTION wrapper for `SaveItemInstance`/`LoadItemInstance`/`SaveInventoryManager`; no dedicated log category; no ability to reorder entries; no per-entry max check on `AddStack` disabling; picking up via `UZWInventoryComponent::SetupInteraction` search path hard-codes `PlayerCharacter(0)` → PlayerState — dedicated server/secondary players not routed through this SystemS path ((single-player assumption).
13. `uBuild.cs` shipping `Slate` + `SlateCore` plus `Engine`/`SlateCore`-only for the editor module with `UnrealEd` in private — normal for an editor module registering AssetTypeActions; keep in mind the crashes when `AssetTools` categories resolve through `LoadModuleChecked<FZWInventoryEditorModule>("ZWInventoryEditor")` (module must be alive when its own AAA asks for it).

---
*Documentation generated from the reconstructed source snapshot under `/opt/data/projectx/ZWSuite-src/ZWInventory`; where a feature is absent from that snapshot, it is marked "brak" (absent) or as a commented-out `@TODO`.*

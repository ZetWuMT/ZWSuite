# ZWScatteringTool — Technical Documentation

> Note: this reproduction of the source tree contains some Polish inline comments ("WIRTUALNA METODA", "FAZA PLANOWANIA", etc.). They are preserved as evidence of the original code language; documentation itself is in English.

## 1. Overview

**ZWScatteringTool** is a runtime-placed gameplay plugin for Unreal Engine that scatters game content across a level at **BeginPlay**, using a *Scatterer / Probe* pattern:

- A **Scatterer** is a logical AActor placed in a map. On `BeginPlay` (server/authority only) it collects all matching **Probe** actors of a configured `ProbeClass`, delegates *planning + spawning* to a virtual hook `PerformScattering(const TArray<AZWScatterProbe*>&)`, and then destroys all probes (cleanup phase common to all scatterers).
- A **Probe** (`AZWScatterProbe`) is an abstract lightweight marker AActor with a `SceneRoot` and an editor-only billboard. Probes carry `FGameplayTagContainer LocationTags`, which entries use to include/exclude locations.

Two base parametrization structs (`FZWScatterEntry`, `FZWEntrySpawnParams`) define a generic entry model (stack amounts per probe, limits, GameplayTag filtering); a shared algorithm `CalculateSpawnsForEntry` distributes random amounts over randomly shuffled valid probes.

Two specializations exist:

1. **LootScattering** — `AZWLootScatterer` + `AZWLootProbe`: spawns world pickups (`AStaticMeshActor` + mesh + `UZWInteractionComponent` + `UZWInventoryComponent` filled with an `FInventoryPickup`). Multiple loot entries can **share** one probe (each probe accumulates a list of pickup templates).
2. **EnemyScattering** — `AZWPawnScatterer` + `AZWPawnProbe`: spawns actor classes (enemies/pawns) at probes. Each probe hosts **at most one enemy**; probes are consumed (removed from the remaining pool) after allocation, so enemy entries compete for locations while loot entries may stack.

Key characteristics:
- Randomized: both the probe order (per entry: `Algo::RandomShuffle` inside `CalculateSpawnsForEntry`) and the entry order (`Algo::RandomShuffle(ScatterEntryTable)` in the loot scatterer) — different loot allocation per run.
- Amount-controlled spawning: per-probe min/max stack, max probes used per entry, and a global per-entry `MaxTotalItems` cap.
- Tag-driven filtering via Unreal Gameplay Tags (`ExclusionTags`, `InclusionTags` vs probe `LocationTags`).
- Server-authoritative: scattering happens only when `HasAuthority()`.

Dependencies: engine modules (Core, GameplayTags, CoreUObject, Engine, Slate, SlateCore) and sibling ZWSuite plugins **ZWInventory** and **ZWInteraction**.

## 2. Metadata (.uplugin)

File: `ZWScatteringTool.uplugin`

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | `ZWScatteringTool` |
| Description | *(empty)* |
| Category | `Other` |
| CreatedBy / CreatedByURL / DocsURL / MarketplaceURL | *(empty)* |
| EnabledByDefault | false |
| CanContainContent | true |
| IsBetaVersion / IsExperimentalVersion | false / false |
| Installed | false |

**Modules (1):**

| Name | Type | LoadingPhase |
|---|---|---|
| ZWScatteringTool | Runtime | Default |

**Plugin references (both enabled):**

| Plugin | Enabled |
|---|---|
| ZWInventory | true |
| ZWInteraction | true |

No `ReferencedPlugins`-style content listed beyond these two.

## 3. Modules (Build.cs) — ZWScatteringTool module

File: `Source/ZWScatteringTool/ZWScatteringTool.Build.cs`
Class: `public ZWScatteringTool : ModuleRules`

- `PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs`
- `PublicIncludePaths`: *(empty — none configured)*
- `PrivateIncludePaths`: *(empty — none configured)*
- `PublicDependencyModuleNames`:
  - `Core`
  - `GameplayTags`
- `PrivateDependencyModuleNames`:
  - `CoreUObject`
  - `Engine`
  - `Slate`
  - `SlateCore`
  - `ZWInteraction`
  - `ZWInventory`
- `DynamicallyLoadedModuleNames`: *(empty — none configured)*

Notes:
- ZWSuite plugin modules `ZWInteraction` and `ZWInventory` are linked **privately**, but the LootScatterer path reaches their types through public headers (`IPickupable.h`, `ZWInteractionComponent.h`), so this module compiles against both.
- `GameplayTags` is a public dependency because `FGameplayTagContainer` fields appear in public `USTRUCT`/`UCLASS` headers.
- Single-module plugin: all gameplay classes live in the one `ZWScatteringTool` module (`ZWSCATTERINGTOOL_API` export macro).

## 4. Publiczny API — klasy (Public API — classes)

All classes below are exported with `ZWSCATTERINGTOOL_API`. Source-of-truth for members is the `Public/` headers listed.

### 4.1 Module class

**`FZWScatteringToolModule`** (`Public/ZWScatteringTool.h` / `Private/ZWScatteringTool.cpp`)
- Base: `IModuleInterface` (plain C++, no `GENERATED_BODY`; implemented with `IMPLEMENT_MODULE(FZWScatteringToolModule, ZWScatteringTool)`)
- `virtual void StartupModule() override;` — empty body (comment placeholder)
- `virtual void ShutdownModule() override;` — empty body
- Notes: defines `LOCTEXT_NAMESPACE "FZWScatteringToolModule"` in the .cpp.

### 4.2 ScatteringBase

#### Struct: `FZWScatterEntry` (`Public/ScatteringBase/ZWScatterer.h`)
- Specifiers: `USTRUCT(BlueprintType)` (note: source literally contains the typo `BLueprintType` — see §8).
- Purpose: generic per-item scatter parameters: how many to place per probe, how many probes to use, total cap, and GameplayTag filtering.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `MinStackPerProbe` | `int32`, default `1` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Amounts", meta=(ClampMin="1"))` | Minimum amount of items in a single spawned pickup |
| `MaxStackPerProbe` | `int32`, default `1` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Amounts", meta=(ClampMin="1"))` | Maximum amount of items in a single spawned pickup |
| `MaxProbesToUse` | `int32`, default `1` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Limits", meta=(ClampMin="1"))` | Maximum number of probes this item can occupy on the level |
| `MaxTotalItems` | `int32`, default `999` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Limits", meta=(ClampMin="1"))` | Absolute maximum amount spawned in total across all probes (comment: even 10 probes × 5 items can be capped to 4 total) |
| `ExclusionTags` | `FGameplayTagContainer` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Filtering")` | If a probe has ANY of these tags, this item will NOT spawn there |
| `InclusionTags` | `FGameplayTagContainer` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot\|Filtering")` | If a probe has ANY of these tags, ONLY THEN this item will spawn there |

#### Struct: `FZWEntrySpawnParams` (`Public/ScatteringBase/ZWScatterer.h`)
- Specifiers: `USTRUCT()` — **empty body**; an empty extension point specialized by Loot/Enemy spawn-param structs.

#### Class: `AZWScatterProbe` (`Public/ScatteringBase/ZWScatterProbe.h`)
- Specifiers: `UCLASS(Abstract)`, `class ZWSCATTERINGTOOL_API AZWScatterProbe : public AActor`
- Purpose: abstract marker actor placed in the level; probes are found by a Scatterer at BeginPlay and destroyed after scattering.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `AZWScatterProbe()` | constructor | public | Creates `USceneComponent` root, editor billboard (see §5) |
| `LocationTags` | `FGameplayTagContainer` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot Probe")` | Tags describing this location; matched against entry inclusion/exclusion tags |
| `EditorBillboard` | `class UBillboardComponent*` | `#if WITH_EDITORONLY_DATA`, `UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Loot Probe")` | Editor visual helper only |

#### Class: `AZWScatterer` (`Public/ScatteringBase/ZWScatterer.h`)
- Specifiers: `UCLASS(Abstract)`, `class ZWSCATTERINGTOOL_API AZWScatterer : public AActor`
- Purpose: abstract scatterer orchestrator; Base class of `AZWLootScatterer` and `AZWPawnScatterer`.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `AZWScatterer()` | constructor | public | Disables ticking (`PrimaryActorTick.bCanEverTick = false`) |
| `ProbeClass` | `TSubclassOf<AZWScatterProbe>` | `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scatter Configuration")` | The probe class to collect from the world (subclasses set this in their constructors) |
| `BeginPlay()` | `virtual void BeginPlay() override` (protected) | — | Calls `Scatter()` only when `HasAuthority()` |
| `Scatter()` | `void Scatter()` (protected) | — | Collects probes of `ProbeClass`, runs `PerformScattering`, destroys all probes |
| `PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes)` | `virtual void` — `PURE_VIRTUAL(AZWScatterer::PerformScattering, )` (protected) | — | Virtual planning+spawn hook subclasses must implement (Polish comment: "Klasy pochodne muszą to zaimplementować") |
| `CalculateSpawnsForEntry(const FZWScatterEntry& Entry, const TArray<AZWScatterProbe*>& AllProbes)` | `TMap<AZWScatterProbe*, int32>` (protected) | — | Universal allocation algorithm: filter → shuffle → random amounts with caps (see §5) |

Implementation note: includes `Algo/RandomShuffle.h`, `Kismet/GameplayStatics.h`, `ScatteringBase/ZWScatterProbe.h`.

### 4.3 LootScattering

#### Struct: `FZWLootScatterEntry` (`Public/LootScattering/ZWLootScatterer.h`)
- Specifiers: `USTRUCT(BlueprintType)`; **inherits `FZWScatterEntry`**.
- Purpose: loot-specific entry adding what to spawn.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| (inherited) | all `FZWScatterEntry` fields | — | See §4.2 |
| `ItemStaticMesh` | `TSoftObjectPtr<UStaticMesh>` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot")` | The StaticMesh to spawn |
| `ItemDefinition` | `TSoftObjectPtr<UZWInventoryItemDefinition>` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Loot")` | The item to spawn (fwd-declared `class UZWInventoryItemDefinition`) |

#### Struct: `FZWLootSpawnParams` (`Public/LootScattering/ZWLootScatterer.h`)
- Specifiers: `USTRUCT()`; **inherits `FZWEntrySpawnParams`**.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `InventoryPickup` | `FInventoryPickup` | `UPROPERTY(Transient)` | Accumulated pickup data (list of templates) for one probe |
| `StaticMesh` | `TSoftObjectPtr<UStaticMesh>` | `UPROPERTY(Transient)` | Last entry's mesh for this probe |

#### Class: `AZWLootScatterer` (`Public/LootScattering/ZWLootScatterer.h`)
- Specifiers: `UCLASS()`, `class ZWSCATTERINGTOOL_API AZWLootScatterer : public AZWScatterer`
- Purpose: spawns loot pickups (static mesh actors carrying inventory content) at loot probes.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `AZWLootScatterer()` | constructor | public | Sets `ProbeClass = AZWLootProbe::StaticClass()` |
| `ScatterEntryTable` | `TArray<FZWLootScatterEntry>` | `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scatter Configuration")` | Loot entries to scatter; renamed from `LootTable` via CoreRedirect (§6) |
| `PerformScattering(...)` | `virtual void PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes) override` (private) | — | Lowers plans to real actors (see §5) |

Source comment: `//@TODO: Add option to not spawn at all and to spawn only a globally limited amount of items`

Includes: `IPickupable.h`, `ZWInventoryComponent.h`, a **relative include of the sibling plugin**: `"../../../../../ZWInteraction/Source/ZWInteraction/Public/ZWInteractionComponent.h"`, `Algo/RandomShuffle.h`, `Engine/StaticMeshActor.h`, `Kismet/GameplayStatics.h`, `LootScattering/ZWLootProbe.h`.

#### Class: `AZWLootProbe` (`Public/LootScattering/ZWLootProbe.h`)
- Specifiers: `UCLASS()`, `class ZWSCATTERINGTOOL_API AZWLootProbe : public AZWScatterProbe`
- Purpose: concrete loot marker; adds no new members.

| Member | Type / Signature | Purpose |
|---|---|---|
| `AZWLootProbe()` | constructor | Empty body |

### 4.4 EnemyScattering

#### Struct: `FZWPawnScatterEntry` (`Public/EnemyScattering/ZWPawnScatterer.h`)
- Specifiers: `USTRUCT(BlueprintType)`; **inherits `FZWScatterEntry`**.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| (inherited) | all `FZWScatterEntry` fields | — | See §4.2 |
| `EnemyClass` | `TSubclassOf<AActor>` | `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy")` | Actor class (enemy/pawn) to spawn at each allocated probe |

#### Struct: `FZWPawnSpawnParams` (`Public/EnemyScattering/ZWPawnScatterer.h`)
- Specifiers: `USTRUCT()`; **inherits `FZWEntrySpawnParams`** — empty body; currently unused extension point (declared but never referenced by the .cpp).

#### Class: `AZWPawnScatterer` (`Public/EnemyScattering/ZWPawnScatterer.h`)
- Specifiers: `UCLASS()`, `class ZWSCATTERINGTOOL_API AZWPawnScatterer : public AZWScatterer`
- Purpose: spawns enemy actors at pawn probes; **exclusive probe occupancy** (one enemy per probe, probes removed from the pool as they are allocated).

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `AZWPawnScatterer()` | constructor | public | Sets `ProbeClass = AZWPawnProbe::StaticClass()` |
| `ScatterEntryTable` | `TArray<FZWPawnScatterEntry>` | `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Scatter Configuration")` | Enemy entries to scatter |
| `PerformScattering(...)` | `virtual void PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes) override` (private) | — | Planning + spawn (see §5) |

#### Class: `AZWPawnProbe` (`Public/EnemyScattering/ZWPawnProbe.h`)
- Specifiers: `UCLASS()`, `class ZWSCATTERINGTOOL_API AZWPawnProbe : public AZWScatterProbe`
- Purpose: concrete enemy marker; adds no data members, but enables per-frame tick.

| Member | Type / Signature | Specifiers | Purpose |
|---|---|---|---|
| `AZWPawnProbe()` | constructor | public | Enables ticking: commented as the standard template note ("turn this off… if you don't need it") — **left ON** |
| `BeginPlay()` | `virtual void BeginPlay() override` (protected) | — | Empty body (only `Super::BeginPlay()`) |
| `Tick(float DeltaTime)` | `virtual void Tick(float DeltaTime) override` (public) | — | Empty body (only `Super::Tick()`) |

### 4.5 Class hierarchy diagram

```
IModuleInterface
└── FZWScatteringToolModule

AActor
├── AZWScatterProbe (Abstract)
│   ├── AZWLootProbe
│   └── AZWPawnProbe        (Tick enabled, unused)
└── AZWScatterer  (Abstract, ProbeClass, Scatter/PerformScattering/CalculateSpawnsForEntry)
    ├── AZWLootScatterer    (+FZWLootScatterEntry / +FZWLootSpawnParams / ScatterEntryTable)
    └── AZWPawnScatterer    (+FZWPawnScatterEntry / +FZWPawnSpawnParams / ScatterEntryTable)

FZWScatterEntry (struct)
├── FZWLootScatterEntry     (+ItemStaticMesh, +ItemDefinition)
└── FZWPawnScatterEntry     (+EnemyClass)

FZWEntrySpawnParams (struct)
├── FZWLootSpawnParams      (+InventoryPickup, +StaticMesh)
└── FZWPawnSpawnParams      (empty)
```

## 5. Implementation (Private)

### 5.1 `ZWScatteringTool.cpp` (`FZWScatteringToolModule`)
- `StartupModule()` / `ShutdownModule()` are intentionally empty — no subsystem registration, editor delegates, or asset scanning at module level. All behavior lives in the runtime actors.
- `LOCTEXT_NAMESPACE` defined and undef'd; `IMPLEMENT_MODULE(FZWScatteringToolModule, ZWScatteringTool)`.

### 5.2 `ScatteringBase/ZWScatterer.cpp` — orchestration
1. **BeginPlay**: `begin` → `if (HasAuthority()) Scatter();` (no scattering on clients; actors to be destroyed never replicate as probes — no replication flags anywhere in plugin classes, no `Replicated` marking; see §8 risk note on networking).
2. **`Scatter()`**:
   - Early-out if `ProbeClass` is null.
   - `UGameplayStatics::GetAllActorsOfClass(this, ProbeClass, FoundActors)` → cast each actor to `AZWScatterProbe` into `AllProbes` (so entries written for a subclass probe still work; the search is by the configured class).
   - Early-out if no probes.
   - Call `PerformScattering(AllProbes)` — subclass planning + spawning.
   - **Cleanup**: loop `AllProbes`; `if (IsValid(Probe)) Probe->Destroy();` — probes are always consumed at the end, regardless of subclass behavior.
3. **`CalculateSpawnsForEntry(Entry, AllProbes)` → `TMap<AZWScatterProbe*, int32>`** (Polish comments describe the intent inline):
   - **Filter**: skip probes failing tag rules:
     - `Entry.ExclusionTags.IsValid() && Probe->LocationTags.HasAny(Entry.ExclusionTags)` → continue (skip).
     - `Entry.InclusionTags.IsValid() && !Probe->LocationTags.HasAny(Entry.InclusionTags)` → continue.
     - Valid probes appended otherwise.
   - `Algo::RandomShuffle(ValidProbes)` — randomized probe order per entry.
   - **Allocation loop** per remaining valid probe:
     - Stop if `ProbesUsedForThisItem >= Entry.MaxProbesToUse`.
     - `AmountToSpawnHere = FMath::RandRange(Entry.MinStackPerProbe, Entry.MaxStackPerProbe)`.
     - Clamp against global cap: if `CurrentTotalSpawned + AmountToSpawnHere > Entry.MaxTotalItems`, reduce to `Entry.MaxTotalItems - CurrentTotalSpawned`.
     - Stop entirely if clamped amount reaches `<= 0` (global cap exhausted).
     - Store `ResultSpawns.Add(TargetProbe, AmountToSpawnHere)`; increment counters.
   - Returns probe→count allocation map. Notes: no width-first spreading beyond one amount per probe; probe pointer is a TMap key (hash ordering), and the map is filled from the shuffled array, so iteration order of allocations is effectively random; amounts are independent draws per selected probe (no guarantee every valid probe is used when `MaxProbesToUse` caps earlier).

### 5.3 `ScatteringBase/ZWScatterProbe.cpp` — probe setup
- `PrimaryActorTick.bCanEverTick = false`; root = `CreateDefaultSubobject<USceneComponent>(FName("SceneRoot"))` assigned to `RootComponent`.
- `#if WITH_EDITORONLY_DATA`: `EditorBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorBillboard"))` attached to `SceneRoot`, sprite loaded via `LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/Ai_Spawnpoint.Ai_Spawnpoint"))`. Not created in packaged builds.

### 5.4 `LootScattering/ZWLootScatterer.cpp`
Inputs/ints:
- In constructor: `ProbeClass = AZWLootProbe::StaticClass()`.
- Fields used (private): `ScatterEntryTable` (from parent class public property).
- Static forward declarations: `AStaticMeshActor::StaticClass()`, `UZWInteractionComponent::StaticClass()`, `UZWInventoryComponent::StaticClass()`.

Algorithm (two-phase pattern planned by Polish comments; "FAZA PLANOWANIA" = plan, "FAZA SPAWNOWANIA" = spawn):
1. **Planning**
   - `PlannedSpawns: TMap<AZWScatterProbe*, FZWLootSpawnParams>`.
   - `Algo::RandomShuffle(ScatterEntryTable)` — reorders the entire loot table each run.
   - For each entry in random order: skip if `Entry.ItemDefinition.IsNull()`.
   - Allocate entry positions via `CalculateSpawnsForEntry(Entry, AvailableProbes)`.
   - For each `(TargetProbe, AmountToSpawn)` allocation:
     - Build `FPickupTemplate NewTemplate; NewTemplate.ItemDef = Entry.ItemDefinition; NewTemplate.StackCount = AmountToSpawn;`
     - `PlannedSpawns.FindOrAdd(TargetProbe).InventoryPickup.Templates.Add(NewTemplate);` — **multiple entries can stack into the same probe**, so one spawned actor may carry several item templates.
     - `PlannedSpawns.Find(TargetProbe)->StaticMesh = Entry.ItemStaticMesh;` — note this **overwrites** the mesh per probe with the last entry that advanced it (a probe holding entries A and B shows B's mesh; see §8 risk).
2. **Spawn**
   - For each planned `PlannedSpawns` pair:
     - `SpawnTransform = Probe->GetActorTransform()`.
     - `GetWorld()->SpawnActorDeferred<AActor>(AStaticMeshActor::StaticClass(), SpawnTransform, this, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn)` — spawn with this scatterer as owner.
     - On success:
       - Obtain `UStaticMeshComponent* StaticMeshComponent = NewPickup->FindComponentByClass<UStaticMeshComponent>()` and if the entry mesh is provided, `StaticMeshComponent->SetStaticMesh(NewStaticMesh)` where `NewStaticMesh = PlannedSpawn.Value.StaticMesh.LoadSynchronous()` (synchronous load from the soft pointer).
       - Add `UActorComponent* NewIntComp = NewPickup->AddComponentByClass(UZWInteractionComponent::StaticClass(), false, FTransform::Identity, true);` then `NewIntComp->RegisterComponent();`.
       - Add `UActorComponent* NewInvComp = NewPickup->AddComponentByClass(UZWInventoryComponent::StaticClass(), false, FTransform::Identity, false);` then register.
       - Cast to `UZWInventoryComponent* InvComp` and call `InvComp->SetPickupInventory(PickupDataToGrant);`.
       - `UGameplayStatics::FinishSpawningActor(NewPickup, SpawnTransform);`.
   - Note the source explicitly says the spawn phase "Pozostaje w 100% z Twojej starej logiki" (restates 1:1 from prior version) — unchanged logic, only the planning part was refactored.

### 5.5 `LootScattering/ZWLootProbe.cpp`
- Only a constructor with empty body; no per-probe extras.

### 5.6 `EnemyScattering/ZWPawnScatterer.cpp`
Inputs/ints:
- In constructor: `ProbeClass = AZWPawnProbe::StaticClass()`.
- Fields used: `ScatterEntryTable` (parent-class public property).
- No gameplay/random includes beyond `EnemyScattering/ZWPawnProbe.h` (RandomShuffle is indirect via base).

Algorithm:
1. `RemainingProbes = AvailableProbes` — local copy of the free probe pool.
2. `PlannedSpawns: TMap<AZWScatterProbe*, TSubclassOf<AActor>>` — map of probe → enemy class to spawn there.
3. **Planning**
   - For each `FZWPawnScatterEntry Entry` in `ScatterEntryTable` (in declared order — **no shuffle of the entry table**, only per-entry probe shuffle inside `CalculateSpawnsForEntry`): skip if `!Entry.EnemyClass`.
   - Allocate via `CalculateSpawnsForEntry(Entry, RemainingProbes)`.
   - For each allocation:
     - `PlannedSpawns.Add(TargetProbe, Entry.EnemyClass);` — one probe, one enemy; a probe's later allocations by the same entry would collide (TMap `Add` overwrites but `CalculateSpawnsForEntry` yields at most one amount per probe thanks to per-entry RandomShuffle). Since `PlannedSpawns` is keyed by probe, a subsequent allocation at the same probe (post-removal impossible — the probe is consumed immediately) is prevented by the next bullet.
     - `RemainingProbes.Remove(TargetProbe);` — **exclusivity**: once a probe is claimed by entry A, entries B, C, … cannot claim it. Polish source comment marks this step: "KLUCZOWY MOMENT".
4. **Spawn**
   - For each planned pair:
     - Build `FActorSpawnParameters SpawnParams; SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn; SpawnParams.Owner = this;`
     - `GetWorld()->SpawnActor<AActor>(ClassToSpawn, Probe->GetActorTransform(), SpawnParams);` — direct spawn (no deferred pattern, unlike loot).
   - Simplicity trade-off: planned `int32` amounts from `CalculateSpawnsForEntry` are discarded — an enemy entry only "uses" the counts to gate how many probes an entry can occupy; the count itself is ignored. `MinStackPerProbe` / `MaxStackPerProbe` / `MaxTotalItems` therefore only gate how many probes are consumed, **not** whether multiple enemies spawn per probe (exactly one enemy per claimed probe regardless of the drawn count ≥ 1).

### 5.7 `EnemyScattering/ZWPawnProbe.cpp`
- Constructor enables tick: `PrimaryActorTick.bCanEverTick = true;` (with the template's standard comment about turning it off).
- `BeginPlay` and `Tick` are empty aside from `Super::` calls — no gameplay behavior; Tick has zero gameplay cost beyond the frame call overhead itself.

### 5.8 Cross-implementation differences (Base vs Loot vs Enemy)

| Aspect | `AZWScatterer` (base) | `AZWLootScatterer` (Loot) | `AZWPawnScatterer` (Enemy) |
|---|---|---|---|
| Probe class | abstract | `AZWLootProbe` | `AZWPawnProbe` |
| Entry struct | `FZWScatterEntry` | `FZWLootScatterEntry : FZWScatterEntry` | `FZWPawnScatterEntry : FZWScatterEntry` |
| Spawn params | `FZWEntrySpawnParams` (empty) | `FZWLootSpawnParams` (FInventoryPickup + mesh) | `FZWPawnSpawnParams` (unused, empty) |
| Entry-table shuffle | n/a (no table in base) | yes — `Algo::RandomShuffle(ScatterEntryTable)` | no |
| Sharing probes | n/a | **shared** — multiple entries' templates stack on one probe | **exclusive** — `RemainingProbes.Remove(claim)` after each entry-claim |
| Spawn target | n/a | `AStaticMeshActor` via `SpawnActorDeferred` + `FinishSpawningActor` | custom `Entry.EnemyClass` via `SpawnActor` |
| Per-probe extras | n/a | `UZWInteractionComponent` + `UZWInventoryComponent` + `SetStaticMesh` | none |
| Uses the int32 amount | n/a | yes — becomes `FPickupTemplate.StackCount` | no — amount only gates probe consumption |
| Late-stage clean-up | destroys all probes | same (from base `Scatter()`) | same (from base `Scatter()`) |

## 6. Configuration (.ini)

File: `Config/DefaultZWScatteringTool.ini`

```
[CoreRedirects]
+PropertyRedirects=(OldName="/Script/ZWScatteringTool.ZWLootScatterer.LootTable",NewName="/Script/ZWScatteringTool.ZWLootScatterer.ScatterEntryTable")
```

- Single redirect: `AZWLootScatterer.LootTable` → `AZWLootScatterer.ScatterEntryTable`, i.e. previously-serialized loot tables still deserialize into the renamed `TArray<FZWLootScatterEntry>` property.
- Same-name `ScatterEntryTable` field is also present on `AZWPawnScatterer`, but no redirect is recorded for it in this file.
- No other config: no `PerObjectConfig`, no Blueprint list, no gameplay-tag IniConfig.

## 7. Dependencies within ZWSuite

- **Hard plugin references (`ZWScatteringTool.uplugin`)**
  - `ZWInventory` — enabled.
  - `ZWInteraction` — enabled.
- **Build.cs private dependencies**
  - `ZWInventory`, `ZWInteraction` (linked privately, headers still public via their own modules).
- **Runtime API usage by `AZWLootScatterer`:**
  - `UZWInventoryItemDefinition` (loot entry payload; fwd-declared in header; fully resolved at load time via `TSoftObjectPtr`).
  - `FPickupTemplate` / `FInventoryPickup` / `IPickupable.h` — loot datatypes (note: the actual entry yaml uses `NewTemplate.ItemDef` and `.Templates`, so the exact ZWInventory shape is expected, not re-implemented locally).
  - `UZWInventoryComponent::SetPickupInventory(const FInventoryPickup&)` — grants the scattered pickup inventory.
  - `UZWInteractionComponent` — appended to the spawned pickup so it becomes interactable.
- **Runtime usage by `AZWPawnScatterer` / `AZWPawnProbe`:** none — the enemy scattering path is self-contained (only engine + GameplayTags).
- **Hard include cross-plugin:** `#include "../../../../../ZWInteraction/Source/ZWInteraction/Public/ZWInteractionComponent.h"` inside `LootScattering/ZWLootScatterer.cpp` — resolves through the relative path from this module's Private folder (`Source/ZWScatteringTool/Private/LootScattering/` → up 5 levels → sibling plugin `ZWInteraction/Source/ZWInteraction/Public/`). Note the depth: `../../../../../` traverses up out of the plugin directory assuming the two plugins live as siblings under one root tree (see §8 risk about relative include fragility).

## 8. Notes / Risks

1. **Single-module, no phase hooks**: `FZWScatteringToolModule` is empty — a plugin that adds no editor hooks and relies purely on AActor lifecycle. If more modules are added later (`Type: Editor`), they would live under the same .uplugin but this rebuild only documents the single current module.
2. **Typo in base struct specifier**: `USTRUCT(BLueprintType)` appears in three headers (`FZWScatterEntry`, `FZWLootScatterEntry`, `FZWPawnScatterEntry`). Unreal's UHT preprocessor matches specifier names case-insensitively here, so it works, but it is cosmetic noise — safe to normalize to `USTRUCT(BlueprintType)`.
3. **Static mesh overwrite in `PerformScattering` (Loot)**: `PlannedSpawns.Find(TargetProbe)->StaticMesh = Entry.ItemStaticMesh;` overwrites with the last entry that allocated this probe. The pickup's *inventory content* correctly keeps all templates, but the *visual mesh* reflects only one entry. If two different-mesh entries share a probe, only one mesh is visible; the other item's presence is reachable only via interaction/inventory UI.
4. **Direct-vs-deferred spawn asymmetry**: Loot uses `SpawnActorDeferred` + `FinishSpawningActor` (needed because components are added pre-`Finish`); Enemy uses plain `SpawnActor`. If enemy AI needs component pre-work the two paths will diverge further.
5. **Enemy scatter ignores the drawn amount**: a per-probe count (say a min/max stack of 3/5) still spawns only one enemy — the draw's purpose is purely probe custom steering. If the intent is to spawn multiple enemies per probe, this must be changed; the risk is silent under-spawning (max stacks hurt more than they spawn).
6. **Entry iteration determinism in EnemyScattering**: only the *probe* order is shuffled inside `CalculateSpawnsForEntry`; `ScatterEntryTable` itself is iterated in declared order (unlike the Loot path which shuffles the table). With a shared free-probe pool this gives first-come-first-served behavior per entry order — documented behavior, not a bug, but worth knowing for reproducibility/loadouts.
7. **Relative include fragility** (`"../../../../../ZWInteraction/..."`): replace with the Build.cs-provided tuple-specific include (`#include "ZWInteraction/Public/ZWInteractionComponent.h"`, which is already available since ZWInteraction is a private dependency) — the five-dot climb is fragile under plugin relocation and harder to read.
8. **No networking / replication**: Scatterers only scatter on the server (`HasAuthority()`), spawned pickups/enemies are not marked for replication here (no AActor replication setup beyond the default). If the project requires pickups to exist on clients, the neighbor plugins' networking guarantees need to be verified (this plugin does not itself expose replicated scatter state). No `Replicated` flag is set anywhere in these files.
9. **Probe lifetime assumptions**: base `Scatter()` destroys *all* found probes of `ProbeClass` — so two different Scatterers sharing a probe class will both destroy the same probe set (double destroy is guarded via `IsValid`); but the scatter of the *first* scatterer to BeginPlay will still see both scatterers' shared probes. Prefer distinct probe classes per scatterer (as the two specializations do).
10. **Projectile-like empty `Tick` on `AZWPawnProbe`**: `PrimaryActorTick.bCanEverTick = true` with empty Tick — harmless on quantity, but easily disabled by the note in the header.
11. **`FZWPawnSpawnParams` unused** and its derived entry/params never used in `ZWPawnScatterer.cpp`; kept as scaffolding mirroring the loot shape.
12. **`MaxTotalItems` exhaustion path**: probe used counts are tracked per entry, not per probe-pool; after a probe is claimed (EnemyScattering only), the same probe won't be revisited; but the Loot path's shared-probe design silently allows repeated claims at the same probe for different entries until `MaxProbesToUse` cuts the entry off. Sums of amounts across entries can therefore exceed expectations per-probe, in the population sense — that is intentional loot behavior (`MaxTotalItems` is the only "global cap").

## 9. File inventory (source-of-truth list)

| Location relative to plugin root | Role |
|---|---|
| `ZWScatteringTool.uplugin` | Plugin descriptor (version, category, deps on ZWInventory/ZWInteraction) |
| `Config/DefaultZWScatteringTool.ini` | CoreRedirect for LootTable rename |
| `Resources/Icon128.png` | Plugin icon (binary; not documented further) |
| `Source/ZWScatteringTool/ZWScatteringTool.Build.cs` | Module build rules |
| `Source/ZWScatteringTool/Public/ZWScatteringTool.h` | Module interface definition |
| `Source/ZWScatteringTool/Private/ZWScatteringTool.cpp` | Module implementation (empty startup/shutdown) |
| `Source/ZWScatteringTool/Public/ScatteringBase/ZWScatterer.h` | `FZWScatterEntry`, `FZWEntrySpawnParams`, `AZWScatterer` |
| `Source/ZWScatteringTool/Public/ScatteringBase/ZWScatterProbe.h` | `AZWScatterProbe` |
| `Source/ZWScatteringTool/Private/ScatteringBase/ZWScatterer.cpp` | Scatter orchestration + allocation algorithm |
| `Source/ZWScatteringTool/Private/ScatteringBase/ZWScatterProbe.cpp` | Probe constructor + editor billboard |
| `Source/ZWScatteringTool/Public/LootScattering/ZWLootScatterer.h` | `FZWLootScatterEntry`, `FZWLootSpawnParams`, `AZWLootScatterer` |
| `Source/ZWScatteringTool/Public/LootScattering/ZWLootProbe.h` | `AZWLootProbe` |
| `Source/ZWScatteringTool/Private/LootScattering/ZWLootScatterer.cpp` | Loot planning/spawn |
| `Source/ZWScatteringTool/Private/LootScattering/ZWLootProbe.cpp` | Loot probe (empty ctor) |
| `Source/ZWScatteringTool/Public/EnemyScattering/ZWPawnScatterer.h` | `FZWPawnScatterEntry`, `FZWPawnSpawnParams`, `AZWPawnScatterer` |
| `Source/ZWScatteringTool/Public/EnemyScattering/ZWPawnProbe.h` | `AZWPawnProbe` |
| `Source/ZWScatteringTool/Private/EnemyScattering/ZWPawnScatterer.cpp` | Enemy planning/spawn |
| `Source/ZWScatteringTool/Private/EnemyScattering/ZWPawnProbe.cpp` | Pawn probe (tick enabled, empty bodies) |

Every source file listed above was read and is reflected in this documentation. *(No further hidden files were present in the plugin directory besides `Icon128.png`.)*

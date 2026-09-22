# ZWStateTree Plugin Documentation

> Auto-generated from the reconstructed plugin sources at
> `ZWSuite-src/ZWStateTree`. Every source file in the plugin was read verbatim
> (`.uplugin`, `Build.cs`, all `Public/*.h` and `Private/*.cpp`).

---

## 1. Overview

**ZWStateTree** is a lightweight, shared-infrastructure Unreal Engine plugin that provides the
common boilerplate for ZW `LocalPlayerSubsystem`s which each drive a single, independent
State Tree instance (e.g. `UZWInputSubsystem` in **ZWInputStateTree** and
`UZWUIStateTreeSubsystem` in **ZWUIStateTree**).

Before this refactor, every such subsystem duplicated the exact same boilerplate:

- owning an `FStateTreeReference` / `FStateTreeInstanceData` pair;
- ticking the tree every frame via `FTickableGameObject`;
- (re)starting the tree once the local player receives a `PlayerController`;
- building an `FStateTreeExecutionContext` before every `Start` / `Tick` / `SendEvent` call.

`UZWStateTreeSubsystemBase` owns that boilerplate once. A derived class only has to answer
two questions:

1. *Which State Tree asset should I run?* → override `GetStateTreeAsset()`
2. *How do I bind my schema's external context data?* → override `BindContextData()`

**Design intent (verbatim from the class doc block):** the plugin lives in its own lightweight
plugin rather than in ZWCore, because ZWCore is the common ancestor of both the ZWInput family
and the ZWUI family, and neither "bare" ZWInput nor "bare" ZWUICore need `StateTreeModule` as a
dependency. Only the optional `*StateTree` plugins (ZWInputStateTree, ZWUIStateTree) depend on
ZWStateTree, so projects that don't use State Tree at all never pull it in.

**Category:** `ZW/StateTree` · **Not meant to be used standalone** — it is a shared foundation
consumed by other optional ZW State Tree plugins (ZWInputStateTree, ZWUIStateTree). Neither
ZWInput nor ZWUICore depend on this plugin, so bare ZWInput/ZWUICore usage is completely
unaffected.

### Example tree-lifecycle flow (base implementation)

```
LocalPlayer created
  └─ Initialize()
       ├─ GetStateTreeAsset()            (override, returns asset or nullptr)
       ├─ StateTreeRef.SetStateTree(asset)
       └─ StateTreeInstanceData.CopyFrom(asset->GetDefaultInstanceData())

LocalPlayer receives PlayerController
  └─ PlayerControllerChanged(PC)
       └─ if PC && asset && instance data non-empty:
            FStateTreeExecutionContext Context(*this, *asset, InstanceData)
            BindContextData(Context, asset)   (override)
            Context.Start()

Every frame (FTickableGameObject)
  └─ Tick(DeltaTime)
       └─ same Context build → BindContextData → Context.Tick(DeltaTime)
```

---

## 2. Metadata (`ZWStateTree.uplugin`)

Source file: `ZWSuite-src/ZWStateTree/ZWStateTree.uplugin`

| Field                | Value |
|----------------------|-------|
| `FileVersion`        | 3 |
| `Version`            | 1 |
| `VersionName`        | `1.0` |
| `FriendlyName`       | `ZWStateTree` |
| `Description`        | "Shared boilerplate for ZW LocalPlayerSubsystems that drive a single State Tree instance (ticking, start/stop, context binding scaffolding). Not meant to be used standalone - it is a shared foundation consumed by other optional ZW State Tree plugins (ZWInputStateTree, ZWUIStateTree). Neither ZWInput nor ZWUICore depend on this plugin, so bare ZWInput/ZWUICore usage is completely unaffected." |
| `Category`           | `ZW/StateTree` |
| `CreatedBy`          | `tiramisoo` |
| `CreatedByURL`       | (brak) — empty string |
| `DocsURL`            | (brak) — empty string |
| `MarketplaceURL`     | (brak) — empty string |
| `EnabledByDefault`   | false |
| `CanContainContent`  | false |
| `IsBetaVersion`      | false |
| `IsExperimentalVersion` | false |
| `Installed`          | false |

### Module registration

| Name         | Type     | LoadingPhase |
|--------------|----------|--------------|
| `ZWStateTree` | `Runtime` | `Default` |

### Plugin dependencies (`.uplugin` → `Plugins`)

| Plugin      | Enabled |
|-------------|---------|
| `StateTree` | true    |

No other plugin references appear here — in particular there is **no** entry for
`ZWInputStateTree` or `ZWUIStateTree` (the dependency runs in the opposite direction: *those*
plugins are expected to depend on this one).

---

## 3. Modules (Build.cs)

Source file: `ZWSuite-src/ZWStateTree/Source/ZWStateTree/ZWStateTree.Build.cs`

**Module count: 1** — `public class ZWStateTree : ModuleRules` defines a single runtime module
named `ZWStateTree` (declared in the `.uplugin` as `Type: Runtime`, `LoadingPhase: Default`).

| Setting | Value |
|---------|-------|
| `PCHUsage` | `ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs` |
| `DynamicallyLoadedModuleNames` | empty (only placeholders comments) |
| `PublicIncludePaths` / `PrivateIncludePaths` | not set (brak) |

### Public dependency modules

| Module             | Purpose |
|--------------------|---------|
| `Core`             | Engine core |
| `GameplayTags`     | `FGameplayTag` used by `SendStateTreeEvent()` |
| `StateTreeModule`  | `UStateTree`, `FStateTreeReference`, `FStateTreeInstanceData`, `FStateTreeExecutionContext` |

### Private dependency modules

| Module        | Purpose |
|---------------|---------|
| `CoreUObject` | `UObject` machinery (`UPROPERTY(Transient)` members) |
| `Engine`      | `ULocalPlayerSubsystem` / engine types |

Notable: the module **does not** depend on `EnhancedInput`, `DeveloperSettings`, `ZWInput`,
`ZWGameplayActions`, `ZWCore`, or any other ZW plugin. This confirms ZWStateTree is a leaf
dependency, deliberately decoupled from the ZWInput and ZWUI plugin families.

---

## 4. Public API — classes

The plugin exposes **4 types** across 2 public headers.

### 4.1 `class FZWStateTreeModule : public IModuleInterface`

Header: `Source/ZWStateTree/Public/ZWStateTree.h`
Implementation: `Source/ZWStateTree/Private/ZWStateTree.cpp`
Loc-text namespace: `FZWStateTreeModule`. Registered via
`IMPLEMENT_MODULE(FZWStateTreeModule, ZWStateTree)`.

Standard empty module shell — `StartupModule()` / `ShutdownModule()` are both empty bodies
(comment-only). Not a UCLASS; plain C++ module class with no exported API macro.

| Member | Signature | Notes |
|--------|-----------|-------|
| `StartupModule` | `virtual void StartupModule() override` | Empty — no bootstrap logic. |
| `ShutdownModule` | `virtual void ShutdownModule() override` | Empty — no cleanup. |

### 4.2 `class UZWStateTreeSubsystemBase` (Abstract)

Header: `Source/ZWStateTree/Public/ZWStateTreeSubsystemBase.h` (export macro `ZWSTATETREE_API`)
Implementation: `Source/ZWStateTree/Private/ZWStateTreeSubsystemBase.cpp`

```
UCLASS(Abstract)
class ZWSTATETREE_API UZWStateTreeSubsystemBase
    : public ULocalPlayerSubsystem
    , public FTickableGameObject
```

Abstract base. There are no `UFUNCTION`s other than the single BlueprintCallable listed below,
and no `USTRUCT`s or `UENUM`s in this plugin (brak).

#### Public interface — overrides

| Member | Signature | Behavior |
|--------|-----------|----------|
| `Initialize` | `virtual void Initialize(FSubsystemCollectionBase& Collection) override` | Calls `Super::Initialize`. Resolves `GetStateTreeAsset()`; if non-null, does `StateTreeRef.SetStateTree(asset)` and `StateTreeInstanceData.CopyFrom(*this, asset->GetDefaultInstanceData())`. Returns early if no asset. |
| `PlayerControllerChanged` | `virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override` | Only (re)starts the tree once the local player actually owns a controller — "mirrors the behavior of the original, hand-written ZWInputSubsystem / ZWUIStateTreeSubsystem". If `NewPlayerController` is set and asset + instance data are valid: builds `FStateTreeExecutionContext`, calls `BindContextData`, then `Context.Start()`. If the controller is lost (`nullptr`), the base class **intentionally does nothing** — a derived class may add `Context.Stop()` there. |
| `Tick` | `virtual void Tick(float DeltaTime) override` | Builds `FStateTreeExecutionContext` and calls `BindContextData` then `Context.Tick(DeltaTime)` if the asset and instance data are valid. Rebuilds the context **every frame** (context is constructed per invocation, not cached). |
| `GetStatId` | `virtual TStatId GetStatId() const override` | `RETURN_QUICK_DECLARE_CYCLE_STAT(UZWStateTreeSubsystemBase, STATGROUP_Tickables)` |
| `IsTickable` | `virtual bool IsTickable() const override` | Returns `!HasAnyFlags(RF_ClassDefaultObject)` to prevent ticking the CDO in the editor. |

#### Public — BlueprintCallable

| Member | UFUNCTION specifiers | Signature | Behavior |
|--------|----------------------|-----------|----------|
| `StartStateTree` | `UFUNCTION(BlueprintCallable, Category = "ZW\|StateTree")` | `void StartStateTree()` | Rebuilds the instance data from the tree's defaults (`StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData())` — "matches the old StartUITree() behavior") and (re)starts execution via a fresh `FStateTreeExecutionContext` + `BindContextData` + `Context.Start()`. Early-outs silently if `StateTreeRef` holds no asset. Use when the tree must be explicitly (re)started outside the normal `PlayerControllerChanged` flow. |

#### Protected — pure virtual contract (derived classes must implement)

| Member | Signature | Doc (condensed) |
|--------|-----------|-----------------|
| `GetStateTreeAsset` | `virtual const UStateTree* GetStateTreeAsset() const PURE_VIRTUAL(UZWStateTreeSubsystemBase::GetStateTreeAsset, return nullptr;)` | Return the State Tree asset this subsystem should run. Typically implemented by reading a `TSoftObjectPtr<UStateTree>` off the derived plugin's own Developer Settings and loading it synchronously. Called once, from `Initialize()`. |
| `BindContextData` | `virtual void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset) PURE_VIRTUAL(UZWStateTreeSubsystemBase::BindContextData, );` | Bind the schema's external data descriptors (subsystem instance, player controller, pawn, etc.) onto the given execution context. Called before **every** Start/Tick/SendEvent. |

#### Protected — non-virtual helper

| Member | Signature | Behavior |
|--------|-----------|----------|
| `SendStateTreeEvent` | `void SendStateTreeEvent(FGameplayTag EventTag)` | Sends a `FGameplayTag` event into the currently running tree, if one is active: builds `FStateTreeExecutionContext`, calls `BindContextData`, then `Context.SendEvent(EventTag)`. Skips silently when asset/data are missing. Note: plain C++ (not a `UFUNCTION`), so it is **not** Blueprint-exposed. |

#### Protected — UPROPERTY state members

| Member | Specifiers | Type | Purpose |
|--------|-----------|------|---------|
| `StateTreeRef` | `UPROPERTY(Transient)` | `FStateTreeReference` | Reference to the State Tree asset this subsystem drives. |
| `StateTreeInstanceData` | `UPROPERTY(Transient)` | `FStateTreeInstanceData` | Working memory (instance data) for the running tree. |

Both are `Transient` and hold neither `EditAnywhere` nor other editor specifiers. There are no
BlueprintAssignable delegates, no config members, and no extra categories beyond
`ZW|StateTree`.

### 4.3 `struct FStateTreeReference` / `FStateTreeInstanceData` usage (engine-owned, referenced)

Not plugin-defined — the plugin merely includes `StateTreeInstanceData.h` and
`StateTreeReference.h`. Forward declarations at the top of the header:

```cpp
class UStateTree;                    // engine StateTreeModule
struct FStateTreeExecutionContext;   // engine StateTreeModule
```

### 4.4 UENUMs / USTRUCTs defined by this plugin

(brak) — the plugin defines no `UENUM` and no plugin-owned `USTRUCT`.

---

## 5. Implementation (Private)

Two private translation units:

### 5.1 `ZWStateTree.cpp`

`Source/ZWStateTree/Private/ZWStateTree.cpp` — module lifecycle for `FZWStateTreeModule`,
described in §4.1. File is 20 lines; both lifecycle hooks are comment-only bodies.
`LOCTEXT_NAMESPACE "FZWStateTreeModule"` is defined and undef'd locally.

### 5.2 `ZWStateTreeSubsystemBase.cpp`

Includes: `StateTree.h`, `StateTreeExecutionContext.h`. Contains the implementation of all six
overridden/helper methods listed in §4.2. Key implementation details worth noting:

- **Rebuild-per-call pattern.** Every operation (`PlayerControllerChanged`, `Tick`,
  `StartStateTree`, `SendStateTreeEvent`) constructs a *fresh*
  `FStateTreeExecutionContext(Context(*this, *TreeAsset, StateTreeInstanceData))`,
  rebinds context data, and acts. The execution context is never stored between frames.
- **Defensive guards.** All five entry points check `TreeAsset` (via `StateTreeRef`) and/or
  `StateTreeInstanceData.Num() > 0` before doing work; all edge cases are silent no-ops.
- **`Initialize`** populates `StateTreeRef` from the virtual asset getter and copies the
  asset's `GetDefaultInstanceData()` into `StateTreeInstanceData` exactly once.
- **`PlayerControllerChanged`** gates on the presence of a controller. The "controller lost"
  branch is deliberately empty (documented in-source), preserving parity with the original
  hand-written subsystems.
- **UKW-stat group.** Tick accounting goes to `STATGROUP_Tickables`.
- Copyright headers: `ZWStateTree.cpp` carries the Epic Games placeholder header, while
  `ZWStateTreeSubsystemBase.cpp` uses the default "Fill out your copyright notice…" header
  (cosmetic inconsistency only).

---

## 6. Configuration (.ini)

(brak) — ZWStateTree contains **no** config classes (`UCLASS(Config=...)` is absent), no
`UDeveloperSettings` subclass, no Default.ini / Engine.ini overrides shipped with the plugin,
and no config-saving references anywhere in its sources. Configuration of the *derived* State
Tree plugins (e.g. `DefaultInputStateTree` in ZWInputStateTree's `UZWInputStateTreeSettings`,
which lives under `Game` config as `"ZW Input"`) is entirely their own responsibility —
ZWStateTree itself is configuration-free.

---

## 7. Dependencies within ZWSuite

### Outbound (what ZWStateTree depends on)

| Dependency | Layer | Evidence |
|-----------|-------|----------|
| `StateTree` plugin | engine plugin | `.uplugin` `Plugins` array (`Enabled: true`); `StateTreeModule` in `Build.cs` public deps; includes of `StateTree.h`, `StateTreeExecutionContext.h` |
| `EnhancedInput` | engine | **none** — not declared anywhere in this plugin |
| Other ZW plugins | — | **none** — `Build.cs` lists no ZW modules and the `.uplugin` names no ZW plugins |

### Inbound (who depends on ZWStateTree)

| Consumer | Evidence |
|----------|----------|
| `ZWUIStateTree` | Referenced throughout as a consumer ("UZWUIStateTreeSubsystem in ZWUIStateTree", "old StartUITree() helper that used to live in ZWUIStateTreeSubsystem"). Source tree for ZWUIStateTree is not present in `ZWSuite-src`, so this reliance is documented at class-comment level rather than verified file-by-file. |
| `ZWInputStateTree` | Intended consumer — **partially migrated: see below and Risks.** |

### Relation to ZWInputStateTree — explicit analysis

Does ZWStateTree depend on ZWInputStateTree? **No.** ZWStateTree never references
`ZWInputStateTree` in code, `.uplugin`, or `Build.cs`; the dependency is conceptually
one-directional (foundation ← specialized plugins).

Does ZWInputStateTree depend on ZWStateTree? **Currently: no, not yet.** Checked evidence:

- `ZWInputStateTree.uplugin` plugin list: `ZWInput`, `EnhancedInput`, `StateTree`,
  `ZWGameplayActions` — **`ZWStateTree` is absent**.
- `ZWInputStateTree.Build.cs`: public deps are `Core`, `GameplayTags`, `EnhancedInput`,
  `StateTreeModule`, `DeveloperSettings`, `ZWInput`, `ZWGameplayActions` — **no `ZWStateTree`
  module**.
- `UZWInputSubsystem` (`ZWInputStateTree/.../Public/ZWInputSubsystem.h`) declares
  `class ZWINPUTSTATETREE_API UZWInputSubsystem : public ULocalPlayerSubsystem, public
  FTickableGameObject` and **does not inherit from `UZWStateTreeSubsystemBase`**; it re-declares
  its own `StateTreeRef`, `StateTreeInstanceData`, Tick/GetStatId/IsTickable overrides and a
  private `BindContextData` — i.e. it is still the *original hand-written duplication* of the
  boilerplate, with expected behavior byte-for-byte identical to the base
  (same `STATGROUP_Tickables`, same `RF_ClassDefaultObject` guard, same
  PlayerControllerChanged `Context.Start()` pattern, same per-op context rebuild).

The header comment of `UZWStateTreeSubsystemBase` states the *refactor goal*:
"`e.g. UZWInputSubsystem in ZWInputStateTree, UZWUIStateTreeSubsystem in ZWUIStateTree`" and
"mirrors the behavior of the original, hand-written ZWInputSubsystem / ZWUIStateTreeSubsystem"
— the mirroring is real and verified, but the actual inheritance withdrawal in ZWInputStateTree
is **not** reflected in its current source. Treat "ZWInputStateTree → ZWStateTree" as a planned
(or regression) wiring, documented in §8.

What ZWStateTree *does* contribute to the ZWInputStateTree direction already:

- Its `SendStateTreeEvent(FGameplayTag)` helper signature matches the way
  `UZWInputSubsystem::ProcessInputTag` routes `FGameplayTag InputTag` into
  `Context.SendEvent(InputTag)` — the same gameplay-tag → tree-event convention.
- The context-binding contract (`GetContextDataDescs()` + `SetContextData(Desc.Handle, …)`
  for `UZWInputSubsystem`, `APlayerController`, and pawns-as-`AActor`) is exactly the pattern
  the base class defers to `BindContextData()`; `UZWInputStateTreeSchema` (ZWInputStateTree)
  registers the three descriptors (`ZWInputSubsystem` @ GUID
  `11111111-2222-3333-4444-444444444444`, `PlayerActor` @
  `55555555-6666-7777-8888-888888888888`, `PlayerController` @
  `22222222-4444-6666-8888-888888888888`) that a future `BindContextData()` override would
  consume.

### Sibling-plugin dependency map (as verified in `ZWSuite-src`)

```
ZWStateTree (this plugin)
 ├── deps:           StateTree plugin (engine)
 ├── consumed by:    ZWUIStateTree (per comments; source absent), ZWInputStateTree (planned — not wired)
 ├── NOT relied upon by: bare ZWInput, bare ZWUICore, ZWGameplayActions, ZWCore
 └── shares runtime model with: ZWInputStateTree's UZWInputSubsystem (hand-written duplicate)
```

---

## 8. Notes / Risks

1. **ZWInputStateTree migration is incomplete / decoupled.** The base-class doc and the
   `.uplugin` `Description` both claim ZWInputStateTree consumes this plugin, but the actual
   `ZWInputStateTree` sources still carry their own full duplicate of the subsystem
   (`UZWInputSubsystem` does not inherit `UZWStateTreeSubsystemBase`, and the plugin does not
   list `ZWStateTree`). Either the migration hasn't landed in this reconstructed snapshot, or
   the `.uplugin` description anticipates a change not yet in code. If a build-step expects
   ZWInputStateTree to link `ZWStateTree`, it will not today; if it should, re-add the module
   to `ZWInputStateTree.Build.cs` and switch the base class. Asymmetric-risk item: nothing
   *breaks* by leaving it as-is, but the "registry"-style promises in the doc comments no
   longer match the code.
   **In summary: no runtime dependency of ZWStateTree on ZWInputStateTree exists; the intended
   reverse dependency (ZWInputStateTree → ZWStateTree) is declared in comments/docs but is not
   wired in the current sources.** The relationship is therefore a *shared convention*
   (eventing + LiveContext binding pattern), not a link-time dependency, until the subsystem
   migration is executed.
2. **Context rebuilt every frame.** `Tick` constructs a fresh `FStateTreeExecutionContext` on
   every tick and re-runs `BindContextData` re-binding all external descriptors. This mirrors
   the per-op semantics of the original hand-written subsystems (and is safe), but it is
   frame-rate scale, not sub-alloc — keep `BindContextData` implementations cheap (GUID /
   descriptor lookups only, no heavy logic).
3. **Silent no-ops on missing data.** All entry points early-out without logging when the
   asset or instance data is missing. Mis-configured assets (e.g. a Developer Settings field
   left null) will produce no visible signal beyond "nothing ticks". Derived classes should
   log/verify in their own `GetStateTreeAsset()` overrides.
4. **No stop-on-controller-loss.** The base class intentionally does nothing when a local
   player loses its controller ("else: optionally Context.Stop() here in a derived class").
   After a controller loss the tree will keep ticking in whatever state it is in; this is
   consistent with the pre-refactor behavior but is worth flagging as a risk in derived
   implementations that expect automatic teardown.
5. **`SendStateTreeEvent` is C++-only.** It is not marked `UFUNCTION`/`BlueprintCallable`
   (unlike `StartStateTree`). Blueprints cannot route tree-events on the base class — a
   derived class must expose its own node if Blueprint eventing is needed (ZWInputStateTree's
   `UZWInputSubsystem::ProcessInputTag` is a `UFUNCTION(BlueprintCallable)` for exactly this
   reason in the sibling plugin, but as a hand-written method rather than an inherited one).
6. **No config surface in this plugin.** All runtime configuration (which tree asset, whether
   schema allows which classes, etc.) lives entirely in the derived *StateTree plugins
   (`ZWInputStateTreeSettings`, etc.). There is no `.ini` shipped here (**brak**).
7. **Module metadata nitpicks.** `DocsURL`, `MarketplaceURL`, `CreatedByURL` are empty;
   `EnabledByDefault` is false (plugin must be explicitly enabled per project/plugin list).
   `CanContainContent` is false — the plugin ships no assets. `Category` is `ZW/StateTree`,
   singular in spirit but groups the whole sub-family of optional zastępcze plugins.
8. **Class-comment mention of "ZWStateTreeCore".** The class doc refers to the plugin as
   "ZWStateTreeCore" in the "Design note" paragraph ("this lives in its own lightweight plugin
   (ZWStateTreeCore), not in ZWCore") while the actual `.uplugin` `FriendlyName`/module is
   named `ZWStateTree`. Treat `ZWStateTreeCore` as an internal/older alias for the same plugin;
   the on-disk plugin name is `ZWStateTree`.
9. **Loading phase is `Default`, `Runtime` type.** Because it is a `LocalPlayerSubsystem`
   ticking via `FTickableGameObject`, it will start ticking only once a `ULocalPlayer` exists
   (subsystem registration is lazy) — safe for client-side usage; the CDO is guarded against
   editor ticking.
10. **License header inconsistency:** `ZWStateTree.cpp` carries the Epic Games copyright
    header, while `ZWStateTreeSubsystemBase.cpp` and `.h` carry the default placeholder header
    ("Fill out your copyright notice…"). Cosmetic only, no functional impact, but worth
    cleaning for going-public of this foundation plugin.

---

*End of documentation. Every source file of the plugin
(`ZWStateTree.uplugin`, `Source/ZWStateTree/ZWStateTree.Build.cs`,
`Source/ZWStateTree/Public/ZWStateTree.h`,
`Source/ZWStateTree/Public/ZWStateTreeSubsystemBase.h`,
`Source/ZWStateTree/Private/ZWStateTree.cpp`,
`Source/ZWStateTree/Private/ZWStateTreeSubsystemBase.cpp`) was read in full and is reflected
in the sections above. Cross-plugin claims were verified against
`ZWSuite-src/ZWInputStateTree` in full (all headers and `.cpp` files read).*

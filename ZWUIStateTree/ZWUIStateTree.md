# ZWUIStateTree — Technical Documentation

> Source analyzed: `/opt/data/projectx/ZWSuite-src/ZWUIStateTree` (16 text/source files + 1 resource icon). All statements below are grounded in the actual code; anything not present in the sources is marked **none (in code)**.

---

## 1. Overview

**ZWUIStateTree** is a runtime-plugin module (category `UI`) that bridges the ZWUICore UI framework with Unreal Engine's **StateTree** system. It provides a dedicated StateTree schema, schema context data (the `UZWUISubsystem` from ZWUICore), a local-player subsystem that owns and ticks the UI state machine, developer settings for assigning the default StateTree asset, and three ready-made StateTree tasks:

| Task (asset display name) | Effect on `EnterState` | Effect on `ExitState` |
|---|---|---|
| `FZWUIStateTreeTask_ManageUI` ("Manage UI Panel") | `UZWUISubsystem::RequestPanelWidget(PanelTag)` | `ClosePanelWidget(PanelTag)` |
| `FZWUIStateTreeTask_ClosePanel` ("Close UI Panel") | `ClosePanelWidget(PanelTag)` (unless deferred) | `ClosePanelWidget(PanelTag)` (when deferred) |
| `FZWUIStateTreeTask_AddInputMappingContext` ("Add Input Mapping Context") | Enhanced Input: `AddMappingContext(IMC, Priority)` | `RemoveMappingContext(IMC)` (if `bRevertOnExit`) |

Architecture (from code comments in the subsystem): the tree acts as the "brain for UI flow, completely decoupled from Input logic." UI events enter the tree as GameplayTags (`ProcessUITag`), driven by `UZWUISubsystem::OnGameplayTagSent` (declared in ZWUICore — implementation lives there).

- **Module name:** `ZWUIStateTree`
- **API macro / export prefix:** `ZWUISTATETREE_API`
- **Module runtime class:** `FZWUIStateTreeModule` (empty `StartupModule` / `ShutdownModule`; `IMPLEMENT_MODULE(FZWUIStateTreeModule, ZWUIStateTree)`)

---

## 2. Metadata (.uplugin) — `ZWUIStateTree.uplugin`

| Field | Value |
|---|---|
| `FileVersion` | 3 |
| `Version` / `VersionName` | 1 / `"1.0"` |
| `FriendlyName` | `ZWUIStateTree` |
| `Description` | `"State Tree framework for ZWUICore plugin."` |
| `Category` | `UI` |
| `CreatedBy` | `tiramisoo` |
| `CreatedByURL` / `DocsURL` / `MarketplaceURL` | empty (`brak` in file) |
| `EnabledByDefault` | false |
| `CanContainContent` | true |
| `IsBetaVersion` / `IsExperimentalVersion` / `Installed` | false |

**Modules (1 total):**

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWUIStateTree` | Runtime | Default |

**Plugin dependencies (3):**

| Plugin | Enabled |
|---|---|
| `ZWUICore` | true |
| `StateTree` | true |
| `EnhancedInput` | true |

**Resources:** `Resources/Icon128.png` (binary; no further metadata extractable).

---

## 3. Sub-modules (Build.cs) — `Source/ZWUIStateTree/ZWUIStateTree.Build.cs`

`public class ZWUIStateTree : ModuleRules` — the plugin contains exactly **one module** (`ZWUIStateTree`); there are no other Build.cs files in the source tree. Key build configuration:

- `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs`
- `PublicIncludePaths` / `PrivateIncludePaths` / `DynamicallyLoadedModuleNames`: empty (template comments only)

**Public dependencies (4):**
`Core`, `StateTreeModule`, `GameplayTags`, `DeveloperSettings`

**Private dependencies (6):**
`CoreUObject`, `Engine`, `Slate`, `SlateCore`, `ZWUICore`, `EnhancedInput`

Note: `StateTree` plugin runtime linkage is achieved through `StateTreeModule` (public); `ZWUICore` and `EnhancedInput` are linked privately, so public ZWUIStateTree headers that reference `ZWUISubsystem.h` (e.g. `Tasks/ZWUIStateTreeTask_ManageUI.h`) rely on the including target also seeing ZWUICore — a minor include-hygiene risk (see §8).

---

## 4. Public API — Classes

Count: **6 documented classes/structs + 1 engine-facing module class + 1 enum + 3 task instance-data structs** (details below). Base classes are taken verbatim from the headers.

### 4.1 `FZWUIStateTreeModule` (`Public/ZWUIStateTree.h`)

- **Base:** `IModuleInterface` (declared without `UCLASS` / `GENERATED_BODY`; plain C++ module class).
- **Members:**
  - `virtual void StartupModule() override` — empty implementation in .cpp.
  - `virtual void ShutdownModule() override` — empty implementation in .cpp.
- **Purpose:** standard UE module bootstrap. Registered with `IMPLEMENT_MODULE`, no custom localization beyond `LOCTEXT_NAMESPACE "FZWUIStateTreeModule"` (defined and #undef'd in the .cpp).

### 4.2 `UZWUIStateTreeSchema` (`Public/ZWUIStateTreeSchema.h`)

- **Base:** `UStateTreeSchema`
- **Specifiers / meta:** `UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "ZWUI State Tree"))`
- **Documented intent (header comment):** schema dedicated to UI management; *requires `UZWUISubsystem` to be provided as context data*.

| UFUNCTION / virtual | Signature |
|---|---|
| ctor | `UZWUIStateTreeSchema()` |
| over-virtual | `virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override` |
| over-virtual | `virtual bool IsClassAllowed(const UClass* InClass) const override` |
| over-virtual | `virtual bool IsExternalItemAllowed(const UStruct& InStruct) const override` |
| over-virtual | `virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override` |

**UPROPERTY:**
- `protected UPROPERTY() TArray<FStateTreeExternalDataDesc> ContextDataDescs;` — populated in the constructor (see §5.2).

### 4.3 `UZWUIStateTreeSubsystem` (`Public/ZWUIStateTreeSubsystem.h`)

- **Base:** `ULocalPlayerSubsystem` **and** `FTickableGameObject` (multiple inheritance; `UCLASS()`, no specifiers).
- **Header comment:** "Subsystem responsible for managing and executing the UI-specific State Tree. It acts as the brain for UI flow, completely decoupled from Input logic."

| Function | Kind | Signature |
|---|---|---|
| `Initialize` | over-virtual | `virtual void Initialize(FSubsystemCollectionBase& Collection) override` |
| `PlayerControllerChanged` | over-virtual | `virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override` |
| `Tick` | over-virtual (`FTickableGameObject`) | `virtual void Tick(float DeltaTime) override` |
| `GetStatId` | over-virtual | `virtual TStatId GetStatId() const override` |
| `IsTickable` | over-virtual | `virtual bool IsTickable() const override` |
| `StartUITree` | plain C++ (no UFUNCTION) | `void StartUITree()` — "Starts the execution of the UI State Tree. Call this from the Player Controller when the UI is ready to be managed." |
| `ProcessUITag` | `UFUNCTION(BlueprintCallable)` | `void ProcessUITag(FGameplayTag UITag)` — "Injects an event tag into the UI State Tree queue", e.g. `UI.Event.Inventory.Closed` |

**UPROPERTY:**
- `private UPROPERTY(Transient) FStateTreeReference StateTreeRef;` — reference to the StateTree asset (assigned via Developer Settings).
- `private UPROPERTY(Transient) FStateTreeInstanceData StateTreeInstanceData;` — "Internal memory storage for the state machine execution."

**Private helper:** `void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset);`

### 4.4 `UZWUIStateTreeSettings` (`Public/ZWUIStateTreeSettings.h`)

- **Base:** `UDeveloperSettings`
- **Specifiers / meta:** `UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZWUI StateTree Settings"))`
- **.hpp forward declaration:** `class UStateTree;`

**UPROPERTY:**
- `public UPROPERTY(Config, EditAnywhere, Category = "StateTree", meta=(AllowedClasses="/Script/StateTreeModule.StateTree")) TSoftObjectPtr<UStateTree> DefaultUIStateTree;` — soft pointer to the default UI StateTree asset.

### 4.5 Task: `FZWUIStateTreeTask_ManageUI` (+ `FZWManageUI_InstanceData`, `EZWUIOperation`) (`Public/Tasks/ZWUIStateTreeTask_ManageUI.h`)

- **Base:** `FStateTreeTaskCommonBase` — `USTRUCT(meta = (DisplayName="Manage UI Panel"))`, `STRUCT` macro `ZWUISTATETREE_API`.
- `using FInstanceDataType = FZWManageUI_InstanceData;` + `override` of `virtual const UStruct* GetInstanceDataType() const`.
- Header also defines:
  - `UENUM(BlueprintType) enum class EZWUIOperation : uint8 { Open ("Open Panel"), Close ("Close Panel") }` — **declared but currently unused**: the corresponding `Operation` UPROPERTY is commented out in the header (lines 36–37). Doc note: enum exists in public API only for future/unused configuration.
  - `USTRUCT() struct FZWManageUI_InstanceData` — empty instance-data struct required so the task compiles in the engine.
- **Configuration UPROPERTYs:**
  - `UPROPERTY(EditAnywhere, Category = "UI") FGameplayTag PanelTag;`
  - `UPROPERTY(EditAnywhere, Category = "UI") bool bRevertOnExit = true;`
- **Overridden task interface:**
  - `virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override`
  - `virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override`

### 4.6 Task: `FZWUIStateTreeTask_ClosePanel` (+ `FZWClosePanel_InstanceData`) (`Public/Tasks/ZWUIStateTreeTask_ClosePanel.h`)

- **Base:** `FStateTreeTaskCommonBase` — `USTRUCT(meta = (DisplayName="Close UI Panel"))`, includes `GameplayTagContainer.h` (and **does not include** `ZWUISubsystem.h`; it forward-use-resolves it in the .cpp).
- Empty `USTRUCT() struct FZWClosePanel_InstanceData` as instance data type.
- **Configuration UPROPERTYs:**
  - `UPROPERTY(EditAnywhere, Category = "UI") FGameplayTag PanelTag;`
  - `UPROPERTY(EditAnywhere, Category = "UI") bool bCloseOnExit = true;`
- **Interface overrides:** `EnterState` / `ExitState` with the same signatures as §4.5.

### 4.7 Task: `FZWUIStateTreeTask_AddInputMappingContext` (+ `FZWAddInputMappingContext_InstanceData`) (`Public/Tasks/ZWUIStateTreeTask_AddInputMappingContext.h`)

- **Base:** `FStateTreeTaskCommonBase` — `USTRUCT(meta = (DisplayName="Add Input Mapping Context"))`. No GameplayTag dependency; class-forward-declares `UInputMappingContext`.
- Empty `USTRUCT() struct FZWAddInputMappingContext_InstanceData`.
- **Configuration UPROPERTYs:**
  - `UPROPERTY(EditAnywhere, Category = "UI") TObjectPtr<class UInputMappingContext> InputMappingContext = nullptr;`
  - `UPROPERTY(EditAnywhere, Category = "UI") bool bRevertOnExit = true;`
  - `UPROPERTY(EditAnywhere, Category = "UI") int32 Priority = 1;`
- **Interface overrides:** `EnterState` / `ExitState`, same signature pattern.

> **Counting note:** named public "classes" (UCLASS + FSTRUCT types + module class + enum) = **11** total: `FZWUIStateTreeModule`, `UZWUIStateTreeSchema`, `UZWUIStateTreeSubsystem`, `UZWUIStateTreeSettings`, `FZWUIStateTreeTask_ManageUI`, `FZWManageUI_InstanceData`, `EZWUIOperation`, `FZWUIStateTreeTask_ClosePanel`, `FZWClosePanel_InstanceData`, `FZWUIStateTreeTask_AddInputMappingContext`, `FZWAddInputMappingContext_InstanceData`.

---

## 5. Implementation (Private)

All private sources live under `Source/ZWUIStateTree/Private/`. No `.ini` configuration files, no `config/Default*.ini` files, no `resources` beyond `Icon128.png` were found in the plugin directory — the section order here follows the call-path of the code rather than per-file location; file origins are cited explicitly.

### 5.1 `Private/ZWUIStateTree.cpp` — module lifecycle (`FZWUIStateTreeModule`)

- `LOCTEXT_NAMESPACE "FZWUIStateTreeModule"` defined and undef'd.
- `StartupModule()` and `ShutdownModule()` are empty; only Unreal-template comments remain. The module therefore contributes no plugin-level editor extension, no delegate registration, no logging of its own (`LogTemp`/`LogZWUI` category: **none (in code)**).

### 5.2 `Private/ZWUIStateTreeSchema.cpp` — schema constructor and allow-listing

Constructor seeds exactly one context descriptor:

```cpp
FStateTreeExternalDataDesc UISubsystemDesc;
UISubsystemDesc.Name = TEXT("UISubsystem");
UISubsystemDesc.Struct = UZWUISubsystem::StaticClass();
UISubsystemDesc.Requirement = EStateTreeExternalDataRequirement::Required;
ContextDataDescs.Add(UISubsystemDesc);
```

Meaning: every StateTree compiled with this schema **must** receive a valid `UZWUISubsystem` context binding at runtime; a tree lacking it fails compile-time validation and would be unusable.

Behavior of the allow-listing overrides:
- `IsStructAllowed`: accepts any `FStateTreeTaskBase`- or `FStateTreeConditionBase`-derived `UScriptStruct` (i.e. "standard StateTree Tasks and Conditions"). Anything else (evaluators, property functions, other custom structs) is rejected unless `Super::` (default engine schema) allows it — actually, `IsStructAllowed` here **returns only the two-task/two-condition check** (no `Super::IsStructAllowed` fallback), so any *non*-base-of-those struct type is excluded from this schema, including third-party task libraries that derive differently.
- `IsClassAllowed`: permits `UZWUISubsystem` subclasses, otherwise defers to `Super::IsClassAllowed(InClass)`.
- `IsExternalItemAllowed`: permits `UZWUISubsystem` subclasses, otherwise defers to `Super::`.
- `GetContextDataDescs`: returns the stored `ContextDataDescs` array (`TConstArrayView`).

Includes pulled in here: `StateTreeConditionBase.h`, `StateTreeTaskBase.h`, `StateTreeTypes.h`, `ZWUISubsystem.h`. `#include "StateTreeLinker.h"` is **not** present in the schema (it is, however, included in each task .cpp, though none of the tasks actually use linker APIs in current code — appears to be a leftover; see §8).

### 5.3 `Private/ZWUIStateTreeSubsystem.cpp` — runtime binding, start, tick, event injection

.Key file; responsibilities synthesized from its 140 lines:

- **`Initialize(FSubsystemCollectionBase&)`**:
  Loads the settings singleton `GetDefault<UZWUIStateTreeSettings>()`, and if `Settings` is valid **and** `Settings->DefaultUIStateTree.IsNull()` is false, loads the tree *synchronously*: `StateTreeRef.SetStateTree(Settings->DefaultUIStateTree.LoadSynchronous());` — hard (blocking) load of the soft pointer; consider streaming risk for large trees (§8).
  Then, if `StateTreeRef.IsValid()` and `TreeAsset` exists, seeds instance storage: `StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());`.
- **`PlayerControllerChanged(APlayerController*)`**:
  - Only when `NewPlayerController != nullptr` (i.e. player *gained* a controller): builds a fresh `FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData)`, calls private `BindContextData(...)` then `Context.Start();`. Guarded by `TreeAsset && StateTreeInstanceData.Num() > 0`.
  - Additionally, if `GetLocalPlayer()` yields a local player, fetches `UZWUISubsystem` via `LP->GetSubsystem<UZWUISubsystem>()` and binds **`UISubsystem->OnGameplayTagSent` → `ProcessUITag(this)`** via `AddUObject`.
  - When the local player *loses* the controller, a comment notes the option of calling `Context.Stop()` here but the code intentionally does **not** implement that reset path (`brak`).
- **Ticking (`FTickableGameObject`)**:
  - `IsTickable()` returns `!HasAnyFlags(RF_ClassDefaultObject)` — i.e. **always ticks for live instances** (no runtime condition like "tree started"), and deliberately excludes the CDO.
  - `GetStatId()` uses `RETURN_QUICK_DECLARE_CYCLE_STAT(UZWUIStateTreeSubsystem, STATGROUP_Tickables);`.
  - `Tick(float DeltaTime)` re-creates a fresh `FStateTreeExecutionContext` **every frame** (guard `TreeAsset && StateTreeInstanceData.Num() > 0`), re-binds context data, then calls `Context.Tick(DeltaTime);`. No early-out when the tree is stopped or not running; the subsystem ticks even during game start before `StartUITree` is called (each frame re-creating a context is a GC/CPU cost — see §8).
- **`BindContextData(FStateTreeExecutionContext&, const UStateTree*)`**:
  - Fetches `TreeAsset->GetContextDataDescs()`.
  - Iterates and, for the first desc where `Desc.Struct->IsChildOf(UZWUISubsystem::StaticClass())`, resolves `UZWUISubsystem` off the local player `GetLocalPlayer()->GetSubsystem<UZWUISubsystem>()`.
  - Wraps the pointer as `FStateTreeDataView UIView(UISubsystem)` and calls `Context.SetContextData(Desc.Handle, UIView);`, then `break;` (only binds the first matching desc).
  - If the local player is null or the subsystem not present, silently leaves the binding unset (schema validation would have caught the missing required desc at compile-time; runtime remains quiet — `brak` of a log/`check` here).
- **`StartUITree()`**:
  Re-copies `GetDefaultInstanceData()`, builds a fresh context, binds, and `Context.Start();`. Designed to be called from the PlayerController when UI is ready to be managed (per header doc comment). No guard against starting twice — calling twice re-copies default data and calls `Start()` again (behavior undefined by this code; engine `FStateTreeExecutionContext::Start` semantics govern).
- **`ProcessUITag(FGameplayTag UITag)`** (the `UFUNCTION(BlueprintCallable)` queued-event entry):
  - Builds a fresh context, `BindContextData(...)`, then `Context.SendEvent(UITag);`.
  - Two commented-out lines show an intended pre-filter: getting `UZWUISubsystem`, and early-returning unless `UISubsystem->IsPanelRegisteredByTag(UITag)`. **Currently disabled** — `ProcessUITag` therefore forwards *every* gameplay tag it receives into the tree (decision risk; see §8).

### 5.4 `Private/ZWUIStateTreeSettings.cpp`

- Exactly one line: `#include "ZWUIStateTreeSettings.h"` — no realizations, no `PostInitProperties`, no `configchanges`/`RegistrationService` customization. The developer-settings entry appears in the editor purely via `UDeveloperSettings` + class meta (section name derived from `DisplayName` "ZWUI StateTree Settings").

### 5.5 `Private/Tasks/ZWUIStateTreeTask_ManageUI.cpp`

```cpp
EnterState:
  if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
    if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
      if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
        UISubsystem->RequestPanelWidget(PanelTag);
        return EStateTreeRunStatus::Running;
  return EStateTreeRunStatus::Failed;
ExitState:
  ... same resolution chain ...
  UISubsystem->ClosePanelWidget(PanelTag);
```

Note: the currently *unused* `bRevertOnExit` property from the header is **never consulted in the .cpp** — `ExitState` always closes the panel regardless of `bRevertOnExit` (flag is dead code at the moment; see §8). Return value on success is `Running` because the owned UI state is expected to persist while the state stays active; the failure path returns `Failed` if any resolution step fails.

### 5.6 `Private/Tasks/ZWUIStateTreeTask_ClosePanel.cpp`

- `EnterState`:
  - If `bCloseOnExit == true` → **early `return EStateTreeRunStatus::Running;`** (defers the actual close to `ExitState`; also means it doesn't attempt the resolution path yet).
  - Otherwise performs the resolve chain: `OwnerSubsystem → GetLocalPlayer → GetSubsystem<UZWUISubsystem>()` and calls `UISubsystem->ClosePanelWidget(PanelTag);` returning `Succeeded` on success, `Failed` if any step fails.
- `ExitState`:
  - If `bCloseOnExit == true`, performs the same resolve chain and calls `UISubsystem->ClosePanelWidget(PanelTag);` (no return semantics — `void`).
  - If `bCloseOnExit == false`, returns immediately (`if (!bCloseOnExit) return;`).
- Note the deliberate `bCloseOnExit` naming (different from `bRevertOnExit` used in the other two tasks): it signals "defer close until exit" rather than "revert on exit."

### 5.7 `Private/Tasks/ZWUIStateTreeTask_AddInputMappingContext.cpp`

- `EnterState`: resolve chain is `OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner())` → `LP = OwnerSubsystem->GetLocalPlayer()` → `UEnhancedInputLocalPlayerSubsystem* EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()` → `EISubsystem->AddMappingContext(InputMappingContext, Priority);` then `return EStateTreeRunStatus::Running;`; failure path returns `Failed`.
- `ExitState`: `if (!bRevertOnExit) return;` — then `EISubsystem->RemoveMappingContext(InputMappingContext);` (no priority. no check that the mapping is actually there; `RemoveMappingContext` handles absence internally in Enhanced Input).
- `InputMappingContext` is dereferenced **without null check** in both `AddMappingContext` and `RemoveMappingContext` calls; Enhanced Input API tolerates a `nullptr` IMC (it's a raw handle) but callers should rely on editor-side validation rather than runtime safety — flagged under §8.

### 5.8 `Private/ZWUIStateTreeSettings.cpp` — see §5.4 (no real logic).

### 5.9 `Private/ZWUIStateTreeSchema.cpp` — see §5.2.

(The two headers `ZWUIStateTree.h` and the three task headers are described in §4; the corresponding .cpp files are described above. `Resources/Icon128.png` has no code-level content readable.)

---

## 6. Configuration (.ini)

- The plugin ships **no .ini files** in its own directory (no `Config/` folder, no `Default*.ini`).
- Runtime configuration lives entirely in project game config via `UZWUIStateTreeSettings` (`Config=Game`, `defaultconfig`) with the single config property `DefaultUIStateTree`:
  - expected in project `DefaultGame.ini` under the auto-generated section `[/Script/ZWUIStateTree.ZWUIStateTreeSettings]` ( typical UDeveloperSettings `defaultconfig` convention — inferred from specifiers; not overridden in code).

---

## 7. Dependencies inside ZWSuite

- **ZWUICore** (direct, via `.uplugin` plugins list and Build.cs `PrivateDependencyModuleNames`):
  - `UZWUISubsystem` is genuinely used, not just mentioned: schema enforces it as required context data (§5.2); subsystem binds its `OnGameplayTagSent` delegate (§5.3 `PlayerControllerChanged`); all three tasks resolve it through `GetLocalPlayer()->GetSubsystem<UZWUISubsystem>()`; and everything eventually funnels to the two real entry points on that API surface:
    - `RequestPanelWidget(PanelTag)` (task ManageUI, EnterState),
    - `ClosePanelWidget(PanelTag)` (tasks ManageUI and ClosePanel).
  - Optional-in-code (currently disabled) `IsPanelRegisteredByTag(UITag)` filter inside `ProcessUITag` (§5.3).
- **StateTree / StateTreeModule** (via Build.cs public dependency and `.uplugin` plugin): the entire plugin's purpose — schema (`UStateTreeSchema`), tasks (`FStateTreeTaskCommonBase`, `FStateTreeTaskBase`), instance data (`FStateTreeInstanceData`), reference (`FStateTreeReference`), execution (`FStateTreeExecutionContext`, `SetContextData`, `SendEvent`, `SendNotification` *not* used), external data (`FStateTreeExternalDataDesc`, `EStateTreeExternalDataRequirement`), asset hooks `GetDefaultInstanceData`, `GetContextDataDescs`, `IsValid`, `GetStateTree`.
- **EnhancedInput** (via `.uplugin` plugin and Build.cs private dependency): used exclusively by the `AddInputMappingContext` task through `UEnhancedInputLocalPlayerSubsystem` (`AddMappingContext` / `RemoveMappingContext`) and the `UInputMappingContext` resource type.
- **Engine gameplay-tag plumbing:** `GameplayTags` (public dep) and `GameplayTagContainer.h` (in `ManageUI` and `ClosePanel` headers/types) — `FGameplayTag PanelTag` on both panel tasks.

---

## 8. Notes / Risks (code-grounded)

1. **Per-frame context construction**: `Tick` (and likewise `ProcessUITag` / `PlayerControllerChanged` / `StartUITree`) constructs a brand-new `FStateTreeExecutionContext` per call and re-binds context data. Combined with `IsTickable()` returning unconditionally `!HasAnyFlags(RF_ClassDefaultObject)` (i.e. always true for live instances, even before the tree is started), every frame pays construction + binding overhead even when the tree is idle or stopped. No early-out for "tree not currently started" exists (`brak`).
2. **`ProcessUITag` has no tag filtering**: the commented-out `IsPanelRegisteredByTag` early-return (§5.3) means any tag injected into `OnGameplayTagSent` reaches the tree. This widens event blast radius — a gameplay tag intended for a *panel* would also be seen by *tree* tasks/conditions if the tree subscribes to shared tags; consider re-enabling the filter or documenting the contract.
3. **`FZWUIStateTreeTask_ManageUI::bRevertOnExit` is dead code**: declared and exposed but never consulted in the .cpp; `ExitState` unconditionally calls `ClosePanelWidget(PanelTag)`. Error-of-intent risk (docs vs. behavior) — either implement the check or remove the UPROPERTY.
4. **`EZWUIOperation` is declared but unused**: `Open`/`Close` enum with a commented-out `UPROPERTY` on `FZWUIStateTreeTask_ManageUI`; dead API surface from a design iteration ("Manage vs. dedicated Close" — the dedicated `ClosePanel` task covers the Close case).
5. **Include hygiene / module-layering asymmetry**: `Source/.../Public/Tasks/ZWUIStateTreeTask_ManageUI.h` directly `#include "ZWUISubsystem.h"`, but `ZWUICore` is only a *private* build dependency; any external module including this public header needs ZWUICore in its own dependency set or the include breaks. `FZWUIStateTreeTask_ClosePanel.h` gets this right (no ZWUICore include in the header; .cpp resolves it).
6. **Synthetic context binding resilience**: `BindContextData` binds only the *first* matching desc and silently no-ops if the local player or subsystem is unavailable (no `check`/log). Schema-level compile validation (`Requirement = Required`) mitigates missing bindings, but a runtime-removal edge (subsystem unregistered) would be silent.
7. **`StartUITree` lacks a re-entry guard**: calling it twice re-copies default instance data and calls `Context::Start()` again; no `IsStarted()`-style protection. Whether double-start is safe depends on engine-side `FStateTreeExecutionContext` semantics, not on this plugin.
8. **`AddInputMappingContext` dereferences `InputMappingContext` without a null check** both on `EnterState` and `ExitState`. Enhanced Input API tolerates some null handling downstream, but relying on engine internals instead of an explicit `IsValid()` guard is fragile.
9. **`ClosePanel` combines two semantics**: `bCloseOnExit == true` defers closing entirely to Exit; `bCloseOnExit == false` closes immediately on Enter and skips Exit work. Both branches reuse the same underlying `ClosePanelWidget(PanelTag)` call — semantically, `bCloseOnExit == true` on this task is *deferred close*, not "close on exit of the containing state," worth documenting for content authors to avoid confusion with `bRevertOnExit`.
10. **Static project settings only**: the tree reference lives in `Config=Game` developer settings (game-wide), no per-player or per-mode override exists in code (`brak`).
11. **No .ini, no editor extensions, no custom logging** shipped by the module; the plugin is runtime-only. `LOCTEXT_NAMESPACE` suggests intended localization string support was stubbed but is currently unused.
12. **`StateTreeLinker.h` is included by all three task .cpp files but no linker APIs are referenced** — likely inherent template residue; harmless but adds a compile dependency.
13. **No tests, no editor-only module** (`Type: Runtime`), no `DynamicallyLoadedModuleNames` in use; the plugin has no loading-phase edge cases (Default only).

---

*End of documentation — generated from the 16 readable source files under `/opt/data/projectx/ZWSuite-src/ZWUIStateTree`.*

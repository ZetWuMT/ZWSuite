# ZWInputStateTree — Technical Documentation

## 1. Overview

ZWInputStateTree is a ZWSuite plugin that bridges ZW's EnhancedInput-based input system to Unreal Engine's StateTree. It provides:

- A custom `UStateTreeSchema` (`UZWInputStateTreeSchema`) that declares the external context data a per-player "input" StateTree may reference (the `UZWInputSubsystem`, the player's `AActor`/Pawn, and the `APlayerController`).
- A `ULocalPlayerSubsystem` (`UZWInputSubsystem`) that owns, ticks, and drives a single `UStateTree` instance: it copies the asset's default instance data, binds context data on every execution, forwards input tags as StateTree events, and also forwards them via a multicast delegate.
- StateTree tasks for GameplayActions (`FZWStateTreeTask_AddGameplayAction`, `FZWStateTreeTask_RemoveGameplayAction`) that grant/remove `UZWGameplayAction` from an actor's `UZWActionManagerComponent` `EnterState`/`ExitState`.
- A config class `UZWInputStateTreeSettings` (extending `UZWInputSettings` from the ZWInput plugin) that selects the default input StateTree asset.

Design intent (from code comments): input events are consumed by StateTree states rather than scattered Blueprint logic; the input-tag delegate is explicitly marked a **temporary solution** ("THIS IS A TEMPORARY SOLUTION. IDEALLY WE'LL ROUTE IT THROUGH UI STATE TREE"). Source comments are in Polish; this document is written from the code, which contains no class-level comments ("Fill out your copyright notice" placeholders).

## 2. Metadata (.uplugin)

| Field | Value |
|---|---|
| File | `ZWInputStateTree.uplugin` |
| FileVersion | 3 |
| Version | 1 (VersionName `1.0`) |
| FriendlyName | `ZWInputStateTree` |
| Description | *(empty)* |
| Category | `Other` |
| CreatedBy / CreatedByURL / DocsURL / MarketplaceURL | *(empty)* |
| EnabledByDefault | `false` |
| CanContainContent | `true` |
| IsBetaVersion | `false` |
| IsExperimentalVersion | `false` |
| Installed | `false` |

**Modules (1):**

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWInputStateTree` | Runtime | Default |

**Plugin dependencies (.uplugin `Plugins`, all `Enabled: true`):** `ZWInput`, `EnhancedInput`, `StateTree`, `ZWGameplayActions`.

## 3. Submodules (Build.cs)

Single module: `Source/ZWInputStateTree/ZWInputStateTree.Build.cs` (rules class `ZWInputStateTree : ModuleRules`).

- `PCHUsage = UseExplicitOrSharedPCHs`
- **PublicDependencyModuleNames:** `Core`, `GameplayTags`, `EnhancedInput`, `StateTreeModule`, `DeveloperSettings`, `ZWInput`, `ZWGameplayActions`
- **PrivateDependencyModuleNames:** `CoreUObject`, `Engine`, `Slate`, `SlateCore`
- Public/Private include paths and `DynamicallyLoadedModuleNames`: empty.

Directory layout: `Public/` holds `ZWInputStateTree.h`, `ZWInputStateTreeSchema.h`, `ZWInputSubsystem.h`, `ZWInputStateTreeSettings.h`, `ZWStateTreeTasks_GameplayActions.h`; `Private/` holds the matching five `.cpp` files (`ZWInputStateTreeSettings.cpp` includes its header only).

## 4. Public API — Classes

### 4.1 `FZWInputStateTreeModule` (runtime module)

- File: `Public/ZWInputStateTree.h` / `Private/ZWInputStateTree.cpp`
- Base: `IModuleInterface`
- API: `virtual void StartupModule() override;` `virtual void ShutdownModule() override;`
- Purpose: standard module shell — both functions are empty (comments only). Instantiated via `IMPLEMENT_MODULE(FZWInputStateTreeModule, ZWInputStateTree)` with `LOCTEXT_NAMESPACE "FZWInputStateTreeModule"`.

### 4.2 `UZWInputStateTreeSchema : UStateTreeSchema` (`UCLASS()`, `ZWINPUTSTATETREE_API`)

- File: `Public/ZWInputStateTreeSchema.h` / `Private/ZWInputStateTreeSchema.cpp`
- Constructor populates `ContextDataDescs` with three `FStateTreeExternalDataDesc` entries (hard-coded GUIDs):

| Name | Class | GUID |
|---|---|---|
| `ZWInputSubsystem` | `UZWInputSubsystem` | `11111111-2222-3333-4444-444444444444` |
| `PlayerActor` | `AActor` | `55555555-6666-7777-8888-888888888888` |
| `PlayerController` | `APlayerController` | `22222222-4444-6666-8888-888888888888` |

- API members:
  - `virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override { return ContextDataDescs; }` — inline, returns the descriptors.
  - `virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;` — protected. Allows structs that derive from `FStateTreeTaskBase`, `FStateTreeConditionBase`, or `FStateTreeEvaluatorBase` (i.e., the base StateTree node types); everything else rejected.
  - `virtual bool IsClassAllowed(const UClass* InClass) const override;` — protected. Null class → `false` (note: a pin-clear operation passes a null class, and this code explicitly returns false for it, contrary to a comment claiming the engine requires it to pass). Allows subclasses of `UZWInputSubsystem`, `AActor`, and `APlayerController`; otherwise defers to `Super::IsClassAllowed`.
  - `UPROPERTY() TArray<FStateTreeExternalDataDesc> ContextDataDescs;` — protected storage for the three descriptors.
- Purpose: validates which structs/tasks and classes are usable inside an input-flavored StateTree and defines what context data the runtime must supply.

### 4.3 `UZWInputSubsystem : ULocalPlayerSubsystem, FTickableGameObject` (`UCLASS()`, `ZWINPUTSTATETREE_API`)

- File: `Public/ZWInputSubsystem.h` / `Private/ZWInputSubsystem.cpp`
- Purpose: per-local-player owner and driver of the input StateTree, plus thin wrappers around the EnhancedInput mapping-context stack.
- Delegates:
  - `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputTagDelegate, FGameplayTag, SignalTag)` — declared at file top with the "temporary solution" note.
  - `UPROPERTY(BlueprintAssignable, Category = "ZW|Input") FOnInputTagDelegate OnInputTagDelegate;` — broadcast on every processed input tag, regardless of whether a tree is bound.
- Public methods (all `UFUNCTION(BlueprintCallable, Category = "ZWInput")` where marked):
  - `virtual void Initialize(FSubsystemCollectionBase& Collection) override;`
  - `virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;`
  - `virtual void Tick(float DeltaTime) override;` / `virtual TStatId GetStatId() const override;` / `virtual bool IsTickable() const override;` — `FTickableGameObject` implementation; `GetStatId` returns `STATGROUP_Tickables`; `IsTickable` is `!HasAnyFlags(RF_ClassDefaultObject)` (always ticks for the real instance, even when no tree — see ⚠ §8).
  - `void ProcessInputTag(FGameplayTag InputTag, const FInputActionValue& InputActionValue);` — builds an execution context, binds context data, `Context.SendEvent(InputTag)`, then broadcasts `OnInputTagDelegate(InputTag)`. (The `FInputActionValue` parameter is accepted but unused in the body.)
  - `void PushInputContext(const UInputMappingContext* IMC, int32 Priority);` — adds the IMC to `UEnhancedInputLocalPlayerSubsystem` and records it in `ActiveContexts` (via `AddUnique`).
  - `void PopInputContext(const UInputMappingContext* IMC);` — removes the IMC and drops it from `ActiveContexts`.
- Properties:
  - `UPROPERTY(EditAnywhere, Category = "Input") FStateTreeReference StateTreeRef;` — the tree to run (normally assigned from settings at `Initialize`).
  - `UPROPERTY(Transient) FStateTreeInstanceData StateTreeInstanceData;` — working memory of the tree.
  - `UPROPERTY(Transient) TArray<TObjectPtr<const UInputMappingContext>> ActiveContexts;` (private) — IMCs this subsystem pushed itself, so it can clean up.
- Private: `void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset);` — see §5.

### 4.4 StateTree GameplayAction tasks (`Public/ZWStateTreeTasks_GameplayActions.h`, `Private/...cpp`)

Both are `USTRUCT`, `meta = (Category = "ZW Actions")`, `ZWINPUTSTATETREE_API`, deriving from `FStateTreeTaskBase` with `GetInstanceDataType()` overridden to return their instance-data `UStruct`. Each calls `TargetActor->GetComponentByClass<UZWActionManagerComponent>()` and returns `EStateTreeRunStatus::Running` on enter.

**`FZWStateTreeTask_AddGameplayAction`** — DisplayName **"Add Gameplay Action"**.
- Instance data `FZWStateTreeTask_AddGameplayAction_InstanceData` (`USTRUCT`, `GENERATED_BODY`):
  - `UPROPERTY(EditAnywhere, Category = "Input") AActor* TargetActor = nullptr;` — action recipient (typically wired from context data).
  - `UPROPERTY(EditAnywhere, Category = "Parameter") TSubclassOf<UZWGameplayAction> ActionClass;`
  - `UPROPERTY(EditAnywhere, Category = "Parameter") bool bRemoveOnExit = true;`
- `virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;` — grants `ActionClass` on the target's `UZWActionManagerComponent` via `GrantAction(...)`.
- `virtual void ExitState(...) const override;` — if `bRemoveOnExit` and both fields valid, calls `RemoveAction(...)` on the same component.

**`FZWStateTreeTask_RemoveGameplayAction`** — DisplayName **"Remove Gameplay Action"**.
- Instance data `FZWStateTreeTask_RemoveGameplayAction_InstanceData`: same `TargetActor` (`TObjectPtr<AActor>` here) and `ActionClass` fields; no exit-cleanup flag.
- `virtual EStateTreeRunStatus EnterState(...) const override;` — calls `RemoveAction(...)`; no `ExitState` override (nothing to undo).

Header note: `#include "ZWGameplayAction.h" // from the other plugin!` — these tasks directly couple this plugin to `ZWGameplayActions`.

### 4.5 `UZWInputStateTreeSettings : UZWInputSettings` (`UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Input"))`, `ZWINPUTSTATETREE_API`)

- File: `Public/ZWInputStateTreeSettings.h` / `Private/ZWInputStateTreeSettings.cpp` (the .cpp only includes the header).
- Purpose: project config (Game config / `defaultconfig`) exposing which StateTree asset the input subsystem should load by default.
- Members:
  - `UPROPERTY(Config, EditAnywhere, Category = "StateTree", meta=(AllowedClasses="/Script/StateTreeModule.StateTree")) TSoftObjectPtr<UStateTree> DefaultInputStateTree;`
- Base: extends `UZWInputSettings` **from the ZWInput plugin** (same plugin's settings file, one common config save; `#include "ZWInputSettings.h"`).

## 5. Implementation (Private)

Key behaviors in the private sources:

- **`UZWInputSubsystem::Initialize`**: reads `GetDefault<UZWInputStateTreeSettings>()`; if `DefaultInputStateTree` is non-null, `LoadSynchronous()` (one-time synchronous load at player startup — noted in comments) and `StateTreeRef.SetStateTree(LoadedTree)`. If `StateTreeRef` is valid, `StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData())` seeds per-instance working data. No validation of the loaded tree beyond null checks.
- **`PlayerControllerChanged`**: when a new `APlayerController` arrives, finds the controller's `UZWInputComponent` (from the ZWInput plugin) and binds `OnInputTagTriggered` → `ProcessInputTag`. Then, if instance data exists, builds an `FStateTreeExecutionContext(*this, *TreeAsset, StateTreeInstanceData)`, `BindContextData(...)`, and `Context.Start()`. When the controller is lost, a comment notes `Context.Stop()` would be an *optional* cleanup but **is not implemented** — old state is not reset (see §8).
- **`Tick`**: if tree + instance data, builds a fresh execution context each tick, binds context data, `Context.Tick(DeltaTime)`. (A new context per execution, calling `Start()` once and re-binding — see §8 for lifecycle caveats.)
- **`ProcessInputTag`**: as described in §4.3 — event dispatch into StateTree, delegate broadcast.
- **`PushInputContext`/`PopInputContext`**: null guard, then `AddMappingContext(IMC, Priority)` / `RemoveMappingContext(IMC)` on `UEnhancedInputLocalPlayerSubsystem` retrieved via `GetLocalPlayer()->GetSubsystem<...>()`. `PushInputContext` tracks chips in `ActiveContexts`; `PopInputContext` removes them.
- **`BindContextData(FStateTreeExecutionContext&, const UStateTree*)`**: the critical linkage. Iterates `TreeAsset->GetContextDataDescs()` (the *compiled* tree's descriptors, which the comments stress have valid generated handles); for each:
  - Descriptor expecting `UZWInputSubsystem` (or subclass) → `Context.SetContextData(Desc.Handle, FStateTreeDataView(this))`.
  - Expecting `APlayerController` (or subclass) → `Context.SetContextData(Desc.Handle, FStateTreeDataView(GetLocalPlayer()->GetPlayerController(GetWorld())))`.
  - Expecting `AActor` (or subclass) → resolves the local player's PC → `PC->GetPawn()` and injects the pawn view under that handle.
  Note: the descriptor name is not read — only the `Desc.Struct` class hierarchy is checked, so a second `AActor`-typed context slot would receive the pawn too (same value); and if the pawn is null (no pawn, e.g., unpossessed), the handle is left unset.
- **`ZWStateTreeTasks_GameplayActions.cpp`**: grants/removes actions on `UZWActionManagerComponent` via the component from the ZWGameplayActions plugin, exactly as described in §4.4. No error logging on missing components/actions; missing `TargetActor`/`ActionClass` are silently ignored.
- **`ZWInputStateTreeSchema.cpp`**: constructor fills `ContextDataDescs`; `IsStructAllowed` is base-type-driven; `IsClassAllowed` is subclass-based with a fallback to `Super::IsClassAllowed`.

## 6. Configuration (.ini)

Registered via `UCLASS(Config=Game, defaultconfig)` on `UZWInputStateTreeSettings` → persists in **`DefaultGame.ini`** (Game config category), under the `DisplayName="ZW Input"` settings entry.

Configured value:
- `/Script/ZWInputStateTree.ZWInputStateTreeSettings: DefaultInputStateTree` — `TSoftObjectPtr<UStateTree>` asset path (validated against `AllowedClasses="/Script/StateTreeModule.StateTree"`), loaded synchronously at subsystem `Initialize`.

No other `.ini` keys or code-level config reads exist in this plugin (checked — no engine config section nor console variable is referenced anywhere in the sources).

## 7. Dependencies within ZWSuite

| Dependency | Kind | Usage here |
|---|---|---|
| `ZWInput` plugin | runtime module + `.uplugin` dependency | Provides `UZWInputSettings` (base class of the plugin's own Settings), `UZWInputComponent`, and `UZWInputComponent.OnInputTagTriggered` (subscribed in `PlayerControllerChanged`). Also exposed to `EnhancedInput` plumbing used in `PushInputContext`/`PopInputContext`. |
| `ZWGameplayActions` plugin | runtime module + `.uplugin` dependency | Provides `UZWGameplayAction`, `UZWActionManagerComponent`, and the `GrantAction`/`RemoveAction` methods called by the two StateTree tasks. |
| `EnhancedInput` | engine plugin | `UInputMappingContext`, `UEnhancedInputLocalPlayerSubsystem`, `FInputActionValue`. |
| `StateTree`/`StateTreeModule` | engine | `UStateTreeSchema`, `FStateTreeTaskBase`, `FStateTreeExternalDataDesc`, `FStateTreeExecutionContext`, `FStateTreeReference`, `FStateTreeInstanceData`, etc. |

External (engine) modules used: `Core`, `CoreUObject`, `Engine`, `GameplayTags`, `Slate`/`SlateCore` (Build.cs), plus Engine's `UPlayerController`/`APawn`/`AActor`.

## 8. Notes / Risks

1. **Temporary prototype seams, explicitly flagged in code.** The `FOnInputTagDelegate` broadcast is documented in the header as a temporary solution until input is routed through a UI State Tree; reliance on it is a known refactor risk.
2. **New execution context every event/tick.** `Tick`, `PlayerControllerChanged`, and `ProcessInputTag` each construct a fresh `FStateTreeExecutionContext` and re-call `BindContextData` + either `Start()` or `Tick/SendEvent`. While the instance data survives in `StateTreeInstanceData`, node-instance/task global state (`FStateTreeExecutionContext`-owned linked evaluators, etc.) depends on how the engine blends per-callstate across context instances. If the tree misbehaves on transitions across frames (e.g., `bRemoveOnExit` cleanup not firing on the right context), suspect here first.
3. **Controller loss is a no-op.** `PlayerControllerChanged(nullptr)` does nothing; the commented-out `Context.Stop()` means the StateTree's state is not reset when the player loses control (or on possession changes). Asymmetry: `Start()` has no matching `Stop()`.
4. **Pawn/context fallback gaps.** In `BindContextData`, an unset pawn (or unset player controller) leaves the corresponding context handle unfilled rather than setting a null view — tasks referencing it through that handle can behave unpredictably. Also, multiple descriptors of the same class hierarchy (e.g., two `AActor` slots) would all be bound to the same pawn — no per-name resolution.
5. **Schema `IsClassAllowed` null rejection.** The implementation returns `false` for a null class despite an explanatory comment stating the opposite intent ("engine passes through to clear the pin"). In practice the editor may rely on such a pass-through — worth verifying during a UI State Tree review.
6. **Synchronous asset load at startup.** `LoadSynchronous` on the default tree can add hitch on local-player init; no streaming option is provided, and the `PlayerControllerChanged` route never loads, only re-uses instance data already primed in `Initialize`.
7. **`IsTickable()` unconditional.** The subsystem ticks on every frame the moment it's not a CDO, so an invalid/unused tree still pays per-frame checks (`TreeAsset && `StateTreeInstanceData.Num() > 0` guards) — negligible cost, but a no-tree tick is not gated off.
8. **No logging or error surfacing.** Missing target actors/actions/component failures inside `GrantAction/RemoveAction` are silent; silent failures make it harder to author states against unexpected setup in PIE.
9. **No tests or editor module.** Plugin ships no editor module, no StateTree editor filter/registration glue, and source comments are in Polish with placeholder copyright descriptions — internal-quality code, not shipping-grade docs.
10. **Localization hook unused.** The `LOCTEXT_NAMESPACE` is scoped but never used to tag any string.

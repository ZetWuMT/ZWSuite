# ZWGameplayActions — Technical Documentation

## 1. Overview

**ZWGameplayActions** is an Unreal Engine plugin for the ZWSuite ecosystem implementing a lightweight, GameplayTag-driven player-action system (a minimalist "GAS-lite" ability pattern).

Concept:
- The logic of each concrete action (e.g. jump, interact, take photo) lives in **Blueprints derived from `UZWGameplayAction`**. Each BP has a config field (`TriggerTag`) and an overridable event **`ExecuteAction`** (a `BlueprintNativeEvent`); all logic is authored in Blueprint graph.
- **`UZWActionManagerComponent`** (attached to the player Pawn) holds the "pocket" of granted actions (`GrantedActions`). It grants (`GrantAction`) and removes (`RemoveAction`) actions — per code comments, invoked from InputStateTree Tasks.
- The input router calls `HandleInputTag(InputTag, ActionValue)`; the component matches the incoming tag against each action's `TriggerTag` (`MatchesTagExact`) and invokes the BP logic.

The plugin is content-capable (`CanContainContent: true`) and contains two modules:

| Module | Type | Role |
|---|---|---|
| `ZWGameplayActions` | Runtime | Core: `UZWGameplayAction`, `UZWActionManagerComponent` |
| `ZWGameplayActionsEditor` | Editor | Asset factory + asset definition (color/category in the Content Browser) |

The editor module adds developer UX: right-click in the Content Browser under the **"ZW Framework"** category → **"ZW Gameplay Action"** creates a ready-made Blueprint with the `ExecuteAction` event node automatically placed in the Event Graph (via `FKismetEditorUtilities::AddDefaultEventNode`), with a dedicated red asset color.

Other characteristics:
- Depends on **EnhancedInput** (both as a plugin in `.uplugin` and as a module in the runtime Build.cs).
- No compile-time dependencies on other ZWSuite plugins (see section 7 — integrations are conventions documented in code comments, not link dependencies).
- The component does not tick (`bCanEverTick = false`) — purely event-driven.

## 2. Metadata (.uplugin)

Source: `ZWGameplayActions.uplugin` (FileVersion 3).

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | "1.0" |
| FriendlyName | "ZWGameplayActions" |
| Description | *(empty)* |
| Category | "Other" |
| CreatedBy / CreatedByURL | *(empty)* |
| DocsURL / MarketplaceURL | *(empty)* |
| EnabledByDefault | false |
| CanContainContent | **true** |
| IsBetaVersion / IsExperimentalVersion | false / false |
| Installed | false |

**Modules** (2):

| Name | Type | LoadingPhase |
|---|---|---|
| ZWGameplayActions | Runtime | Default |
| ZWGameplayActionsEditor | Editor | Default |

**Referenced plugins** (1):

| Name | Enabled |
|---|---|
| EnhancedInput | true |

Resources: `Resources/Icon128.png` (plugin icon, no code).

## 3. Submodules (Build.cs)

### 3.1 `Source/ZWGameplayActions/ZWGameplayActions.Build.cs` (Runtime)

- `PCHUsage = UseExplicitOrSharedPCHs`
- PublicIncludePaths / PrivateIncludePaths / DynamicallyLoadedModuleNames: empty (template comments).
- **PublicDependencyModuleNames**: `Core`, `GameplayTags`
- **PrivateDependencyModuleNames**: `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `EnhancedInput`

Note: `EnhancedInput` is a **private** dependency of the runtime module even though the `FInputActionValue` type appears in the public signature of `HandleInputTag` (see section 8, risk #2).

### 3.2 `Source/ZWGameplayActionsEditor/ZWGameplayActionsEditor.Build.cs` (Editor)

- `PCHUsage = UseExplicitOrSharedPCHs`
- **PublicDependencyModuleNames**: `UnrealEd`, `CoreUObject`, `Engine`, `BlueprintGraph`, `KismetCompiler`, `Core`, `AssetDefinition`
- **PrivateDependencyModuleNames**: `ZWGameplayActions` (the runtime module), `Slate`, `SlateCore`, `AssetTools`

Both Build.cs classes are standard `public class ... : ModuleRules` taking `ReadOnlyTargetRules Target`.

## 4. Public API — classes (Runtime module `Source/ZWGameplayActions/Public`)

### 4.1 `UZWGameplayAction` (`ZWGameplayAction.h`)

- **Base:** `UObject`, specifiers: `UCLASS(Blueprintable, Abstract, EditInlineNew)`, API macro `ZWGAMEPLAYACTIONS_API`.
- Purpose: abstract, Blueprint-authorable "container for one player action's logic" — a trigger tag (which input fires it) plus an execution entry point (what it does).

| Kind | Declaration | Notes |
|---|---|---|
| UPROPERTY | `FGameplayTag TriggerTag` | `EditDefaultsOnly`, `BlueprintReadOnly`, Category `"Action Config"`. The tag that triggers the action (e.g. `Input.Action.Interact`). |
| UFUNCTION | `void ExecuteAction(APlayerController* Controller, APawn* Pawn)` | `BlueprintNativeEvent`, Category `"Action Execution"`. Main logic entry point, overridden in BP (event `ExecuteAction`). |
| Method | `virtual UWorld* GetWorld() const override` | Lets BP nodes with world context work (LineTrace, SpawnActor). Implementation: returns `nullptr` for CDOs; otherwise `GetOuter() ? GetOuter()->GetWorld() : nullptr`. |

### 4.2 `UZWActionManagerComponent` (`ZWActionManagerComponent.h`)

- **Base:** `UActorComponent`, `UCLASS(ClassGroup=(ZW), meta=(BlueprintSpawnableComponent))`, API macro `ZWGAMEPLAYACTIONS_API`.
- Purpose: action inventory/manager on the pawn — holds action instances and routes input tags into BP logic. Implements a "grant/remove ability" pattern without the weight of GAS.

Public (all `BlueprintCallable`, Category `"ZW Actions"`):

| Signature | Purpose (per header comments) |
|---|---|
| `void GrantAction(TSubclassOf<UZWGameplayAction> ActionClass)` | Grants an action into the pocket; "used by InputStateTree Tasks". Guarded against duplicate grants of the same class. |
| `void RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass)` | Removes an action (first match of `IsA`). |
| `void HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue)` | Reacts to input; "hooked to the Broadcast from ZWInputComponent". Fires `ExecuteAction` on actions whose `TriggerTag` matches exactly. |

Protected:

| Element | Declaration | Notes |
|---|---|---|
| Constructor | `UZWActionManagerComponent()` | Public; disables ticking (§5.2). |
| Override | `virtual void BeginPlay() override` | Grants `DefaultActions`. |
| UPROPERTY | `TArray<TSubclassOf<UZWGameplayAction>> DefaultActions` | `EditDefaultsOnly`, Category `"ZW Actions"`. Actions the player has from the start (e.g. Jump, Walk). |
| UPROPERTY | `TArray<TObjectPtr<UZWGameplayAction>> GrantedActions` | `Transient`. Instances of currently granted actions (GC-managed). |

Forward declarations in the header: `class UZWGameplayAction;`, `struct FInputActionValue;`

### 4.3 `FZWGameplayActionsModule` (`ZWGameplayActions.h`)

- **Base:** `IModuleInterface` (plain C++, no UCLASS).
- Purpose: standard runtime module plumbing; `StartupModule()` / `ShutdownModule()` are **empty**. Ends with `IMPLEMENT_MODULE(FZWGameplayActionsModule, ZWGameplayActions)`; LOCTEXT namespace `FZWGameplayActionsModule`.

## 5. Implementation (Private)

### 5.1 Runtime module — `Source/ZWGameplayActions/Private`

**`ZWGameplayActions.cpp`**
- `FZWGameplayActionsModule::StartupModule()` and `ShutdownModule()` are empty template bodies; `IMPLEMENT_MODULE(...)` at the end.

**`ZWGameplayAction.cpp`**
- `UZWGameplayAction::ExecuteAction_Implementation(APlayerController* Controller, APawn* Pawn)` — the C++ default does nothing meaningful: prints an on-screen debug message `AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ExecuteAction!!"))`. This is a fallback visible until the event is overridden in a BP.
- `UZWGameplayAction::GetWorld() const` — returns `nullptr` if the object is a CDO (`RF_ClassDefaultObject`); otherwise walks the Outer chain: `GetOuter() ? GetOuter()->GetWorld() : nullptr`. Because `GrantAction` creates instances with the component as Outer (§5.2), a granted action's `GetWorld()` resolves to the pawn's world at runtime.

### 5.2 `ZWActionManagerComponent.cpp`

- **Constructor:** `PrimaryComponentTick.bCanEverTick = false;` — the manager only listens to input events; zero per-frame cost.
- **`BeginPlay()`** — iterates `DefaultActions` and grants each one via `GrantAction`.
- **`GrantAction(TSubclassOf<UZWGameplayAction> ActionClass)`**
  1. Null-check on `ActionClass` → early return.
  2. **Class-level dedup**: iterates `GrantedActions`; if any existing object `IsA(ActionClass)` → return (no duplicate created). Code comment: the player should never hold 5 instances of the "Take Photo" action.
  3. Creates the instance: `NewObject<UZWGameplayAction>(this, ActionClass)` — **Outer = the component** (important for `GetWorld()`).
  4. Appends to `GrantedActions` (UPROPERTY → GC-referenced).
- **`RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass)`**
  1. Null-check → early return.
  2. Iterates `GrantedActions` **from the end** (standard practice when removing from a TArray).
  3. On the first match of `Action->IsA(ActionClass)`: `RemoveAt(i)` then `break` — removes **only the first** match.
  4. No manual destruction — the Unreal Garbage Collector reclaims the object once it leaves the UPROPERTY array (explicit comment in code).
- **`HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue)`**
  1. Validates `InputTag.IsValid()` → early return otherwise.
  2. Iterates all `GrantedActions`; matches `Action->TriggerTag.MatchesTagExact(InputTag)` — exact match, not hierarchical.
  3. For each match: `Cast<APawn>(GetOwner())` → `APlayerController* PC = Cast<APlayerController>(Avatar->GetController())` → calls `Action->ExecuteAction(PC, Avatar)`.
  4. There is no `break` after the first hit: if two granted actions share the same `TriggerTag`, both execute in the same call. `ActionValue` is accepted but **never used** — it is not forwarded to `ExecuteAction`, which has no parameter for it.

### 5.3 Editor module — `Source/ZWGameplayActionsEditor/Private`

**`ZWGameplayActionsEditor.cpp`**
- `FZWGameplayActionsEditorModule::StartupModule()` / `ShutdownModule()` — **empty**.
- `IMPLEMENT_MODULE(FZWGameplayActionsEditorModule, ZWGameplayActionsEditor)`; LOCTEXT namespace `FZWGameplayActionsEditorModule`.
- The factory and asset definition register through normal UCLASS reflection (nothing registered in StartupModule). The factory implementation files are guarded with `#if WITH_EDITOR`; the asset definition header is not (the whole module is editor-only anyway).

**`ZWGameplayActionFactory.h/.cpp`** — asset factory (`#if WITH_EDITOR`)

- **Base:** `UBlueprintFactory`, `UCLASS()`, API macro `ZWGAMEPLAYACTIONSEDITOR_API`.
- **Constructor** configures the base factory:
  - `SupportedClass = UZWGameplayAction::StaticClass();`
  - `ParentClass = UZWGameplayAction::StaticClass();` (created BPs inherit from `UZWGameplayAction`)
  - `bSkipClassPicker = true;` (no class-picker dialog)
  - `bCreateNew = true;` and `bEditAfterNew = true;` (opens the BP editor after creation)
  - `bEditorImport = false;`
- **`FText GetDisplayName() const override`** → `"ZW Gameplay Action"` (the right-click context-menu label).
- **`FString GetDefaultNewAssetName() const override`** → `"NewGameplayAction"`.
- **`UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override`**:
  1. Calls `Super::FactoryCreateNew` and casts the result to `UBlueprint` (a clean BP).
  2. `FBlueprintEditorUtils::FindEventGraph(NewBP)` — locates the Event Graph.
  3. If found, `FKismetEditorUtilities::AddDefaultEventNode(NewBP, EventGraph, FName(TEXT("ExecuteAction")), UZWGameplayAction::StaticClass(), NodePosY /* = 0 */)` — **automatically inserts** the `ExecuteAction` event node into the new BP's graph (it shows greyed-out until the graph is connected/overridden).
  4. Returns the `UBlueprint`.

**`ZWAssetDefinition_GameplayAction.h/.cpp`** — asset definition (metadata in the Content Browser)

- **Base:** `UAssetDefinition` (module `AssetDefinition`, header `AssetDefinitionDefault.h`), `UCLASS()` with API macro `ZWGAMEPLAYACTIONSEDITOR_API`.
- Overrides:
  - **`GetAssetDisplayName()`** → `NSLOCTEXT("AssetDefinition", "AssetDefinition_ZWGameplayAction", "ZW Gameplay Action")` — the tooltip/hover name.
  - **`GetAssetColor()`** → `FLinearColor(0.8f, 0.1f, 0.1f, 1.0f)` — an "aggressive red" asset color (comment cites R 0.8 / G 0.1 / B 0.1).
  - **`GetAssetClass()`** → `TSoftClassPtr<UObject>` wrapping `UZWGameplayAction::StaticClass()` — per the code comment, UE5 automatically colors all Blueprints inheriting from this class.
  - **`GetAssetCategories()`** → a custom right-click-menu category: `FAssetCategoryPath(NSLOCTEXT("AssetDefinition", "ZWFrameworkCategory", "ZW Framework"))` — i.e. the **"ZW Framework"** category, held in a `static const auto Categories = {...}` initializer (initialized once), returned as `TConstArrayView<FAssetCategoryPath>`.

## 6. Configuration (.ini)

**None.** The plugin ships no `Config/*.ini` files. All configuration happens through:
- `DefaultActions` on the component (Details panel),
- `TriggerTag` on gameplay-action Blueprint defaults,
- GameplayTags registered in the **project** (the project's `Config/DefaultGameplayTags.ini`), not in the plugin.

## 7. Dependencies within ZWSuite

| Direction | Dependency | Kind of coupling |
|---|---|---|
| `ZWGameplayActions` (runtime) → other ZW plugins | **none** | Build.cs references only `Core`, `GameplayTags` (public) and `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `EnhancedInput` (private). |
| `ZWGameplayActionsEditor` → `ZWGameplayActions` | **yes (explicit)** | `ZWGameplayActions` is in the editor module's PrivateDependencyModuleNames; both factory and asset definition call `UZWGameplayAction::StaticClass()`. |
| Comment-documented integrations | **convention, not a link** | `GrantAction` — "used by InputStateTree Tasks"; `HandleInputTag` — "hooked to Broadcast from ZWInputComponent". |

Conclusion: `ZWInputComponent` and the **InputStateTree task system** belong to other ZWSuite plugins (outside this directory). The integration works through this plugin's BlueprintCallable API (another component calls `HandleInputTag`, `GrantAction`, `RemoveAction`), not through module dependencies. This plugin can be adopted standalone; the input router must be supplied externally.

`EnhancedInput` is a required plugin in `.uplugin` and a module in Build.cs; `HandleInputTag` takes the EnhancedInput `FInputActionValue`.

## 8. Notes / risks

1. **`.uplugin` metadata is incomplete** — `Description`, `CreatedBy`, `CreatedByURL`, `DocsURL`, `MarketplaceURL` are empty and Category is `"Other"`. Worth filling in before packaging/marketplace distribution.
2. **Public ABI exposes `FInputActionValue` while `EnhancedInput` is a private module dependency.** `HandleInputTag(FGameplayTag, const FInputActionValue&)` leaks an EnhancedInput type in a public header, but the runtime module links EnhancedInput only as a private dependency. It works in practice because the plugin requires EnhancedInput, but the dependency should arguably be **public** so that external C++ modules consuming this public API link cleanly.
3. **`MatchesTagExact`** — exact tag matching, not hierarchical (`Input.Action` does not match `Input.Action.Interact`). Renaming an input tag silently disconnects actions; there is no validation that `TriggerTag`s other than `IsValid()` are performed at grant time.
4. **No `break` after a hit in `HandleInputTag`** — all actions matching the tag fire in a single call. Possibly intentional (chaining), but a frequent source of double-execution bugs.
5. **`ActionValue` is unused** — `HandleInputTag` accepts the input value but cannot pass it on: `ExecuteAction(Controller, Pawn)` has no argument for it. Analog input (axis values, button strength) cannot drive action logic. A likely future API change.
6. **Owner assumptions** — `HandleInputTag` casts `GetOwner()` to `APawn` and then the pawn's controller to `APlayerController`. If the component sits on something other than a pawn (or the pawn is possessed by a non-player controller), the action silently does nothing. There is **no logging** (`UE_LOG`) anywhere in the plugin — failures are silent.
7. **`ExecuteAction_Implementation` uses `GEngine->AddOnScreenDebugMessage`** — fine in Development builds; in Shipping builds the code path exists but the message does not display. Minor; worth replacing with `UE_LOG`.
8. **`GetWorld()` returns `nullptr` for CDOs** — correct and deliberate: class-default objects in Details panels have no world, while runtime instances (Outer = component) resolve their world through the owning pawn. This is what makes world-context Blueprint nodes (LineTrace, SpawnActor) work inside `ExecuteAction` implementations.
9. **Both modules' `StartupModule`/`ShutdownModule` are empty** — factory and asset-definition registration happens via UE reflection (UCLASS discovery), not module startup code; standard pattern for `UAssetDefinition`-based definitions.
10. **Editor Build.cs pulls heavy modules** (`UnrealEd`, `BlueprintGraph`, `KismetCompiler`) — required for `FKismetEditorUtilities::AddDefaultEventNode` and `FBlueprintEditorUtils::FindEventGraph`. Fine, but keep an eye on compile cost; `AssetDefinition`/`AssetTools` are the newer-style dependencies.
11. **`GetAssetCategories()` returns a static local** — correct idiom; the view points at storage that lives for the module's lifetime. Only a single flat "ZW Framework" category is defined (no nesting).
12. **`IsA`-based dedup is hierarchical** — `RemoveAction` removes only the first `IsA` match (fine), but the `GrantAction` guard means an existing instance of a child class also satisfies `IsA(ParentActionClass)`, so granting a parent class is blocked when any child instance is held (and vice versa). Two distinct instances of the same class are impossible by design; parent+child coexistence can also be blocked unexpectedly.
13. **No replication / networking** — `GrantedActions` is `Transient`, unreplicated. The plugin is single-player/local-centric; replicating the action pocket or server-side granting is out of scope and unsupported out of the box.
14. **No .ini / default bindings shipped** — the plugin adds no key bindings; input comes from the project's EnhancedInput Input Mapping Contexts, routed purely by tags.
15. **Factory only creates, never re-imports or validates** — `bEditorImport=false`; no editor icon logic beyond the asset-definition color; the editor module does not reference `Resources/Icon128.png`.

---

*End of documentation. 2 modules; classes: UZWGameplayAction, UZWActionManagerComponent, FZWGameplayActionsModule, UZWGameplayActionFactory, UZWAssetDefinition_GameplayAction, FZWGameplayActionsEditorModule.*

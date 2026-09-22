# ZWUIHUD — Technical Documentation

## 1. Overview

ZWUIHUD is a small runtime UI plugin of the ZWSuite ("ZW") plugin family. Per its `.uplugin` description it is a **"Decoupled UI HUD framework based on CommonUI and ZWUICore."**

The framework is built around three concepts:

- **HUD Panel** (`UZWUIHUDPanel`) — an abstract, HUD-oriented UI panel. It derives from `UZWUIPanel` (the ZWUICore base panel, itself a `UCommonActivatableWidget`). When constructed, it walks its own `WidgetTree` and initializes every `UZWUIHUDElementSlot` it finds.
- **HUD Element** (`UZWUIHUDElement`) — a `UUserWidget` representing one discrete HUD widget (health bar, compass, kill feed, etc.), spawned at runtime by slots. It exposes a single initialization entry point taking the owning `AActor`.
- **HUD Element Slot** (`UZWUIHUDElementSlot`) — a `UOverlay`-derived container widget that the designer drops into a panel blueprint. It holds (via a soft class reference) which `UZWUIHUDElement` class to spawn, plus the slot layout settings (padding and horizontal/vertical alignment). On `InitializeHUDSlot()` it synchronously loads the element class, creates the widget, adds it to the overlay and applies the layout settings, guarded by an idempotency flag.

The division of labor: the **panel** owns the layout and triggers initialization; the **slot** knows which element class and where/how to place it; the **element** is the actual gameplay-facing widget. Decoupling comes from slots referencing elements via `TSoftClassPtr` (no hard asset references) and from elements receiving their owner actor at init time.

## 2. Metadata (.uplugin)

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | ZWUIHUD |
| Description | Decoupled UI HUD framework based on CommonUI and ZWUICore. |
| Category | UI |
| CreatedBy | tiramisoo |
| CreatedByURL | (empty) |
| DocsURL | (empty) |
| MarketplaceURL | (empty) |
| EnabledByDefault | false |
| CanContainContent | true |
| IsBetaVersion | false |
| IsExperimentalVersion | false |
| Installed | false |

Modules (1):

| Name | Type | LoadingPhase |
|---|---|---|
| ZWUIHUD | Runtime | Default |

Plugin dependencies (3): `CommonUI` (enabled), `EnhancedInput` (enabled), `ZWUICore` (enabled).

## 3. Sub-modules (Build.cs)

Single module: `Source/ZWUIHUD/ZWUIHUD.Build.cs` — class `ZWUIHUD : ModuleRules`.

- `PCHUsage = UseExplicitOrSharedPCHs`
- PublicIncludePaths: none (empty placeholder)
- PrivateIncludePaths: none (empty placeholder)
- **PublicDependencyModuleNames**: `Core`, `CoreUObject`, `Engine`, `InputCore`, `UMG`, `CommonUI`, `GameplayTags`, `ZWUICore`
- **PrivateDependencyModuleNames**: `CoreUObject`, `Engine`, `Slate`, `SlateCore`
- DynamicallyLoadedModuleNames: none

Note: `CoreUObject` and `Engine` appear in both public and private lists (redundant but harmless). `GameplayTags` supports the tag-based panel machinery inherited from `UZWUIPanel`.

## 4. Public API — Classes

All classes live in the runtime module `ZWUIHUD` (API macro `ZWUIHUD_API`). 4 classes total (including the module class).

### 4.1 `FZWUIHUDModule` — module entry
- File: `Public/ZWUIHUD.h`, `Private/ZWUIHUD.cpp`
- Base: `IModuleInterface`
- Methods:
  - `virtual void StartupModule() override` — empty body (comment placeholder only).
  - `virtual void ShutdownModule() override` — empty body (comment placeholder only).

### 4.2 `UZWUIHUDPanel` — HUD panel (abstract)
- File: `Public/ZWUIHUDPanel.h`, `Private/ZWUIHUDPanel.cpp`
- Base: `UZWUIPanel` (from **ZWUICore**; that base is a `UCommonActivatableWidget` carrying `PanelIdentityTag`, `BoundPanelTag`, `bRequiresBackground`, `bRequiresInput`, input config handling, etc. — see §7)
- Class specifier: `UCLASS(Abstract)` — instantiated only via Blueprint subclasses.
- Forward-declared (inherited context): `UZWUIRootLayout`, `UZWUIPlayerHUBWidget` — declared but **not referenced anywhere** in this header or implementation.
- Overrides:
  - `virtual void NativeConstruct() override` (protected)
  - Behavior: calls `Super::NativeConstruct()`, then if `WidgetTree` is valid runs `WidgetTree->ForEachWidget(...)`, and for every widget that `Cast<UZWUIHUDElementSlot>` succeeds on calls `HUDSlot->InitializeHUDSlot()`. A commented-out `UE_LOG(LogTemp, Log, ...)` line (Polish comments in source) exists for debugging slot discovery.
- No UPROPERTYs or UFUNCTIONs declared in this class beyond the override.

### 4.3 `UZWUIHUDElementSlot` — self-initializing slot widget
- File: `Public/ZWUIHUDElementSlot.h`, `Private/ZWUIHUDElementSlot.cpp`
- Base: `UOverlay` (UMG panel widget)
- Category for all members: `"ZWUIHUDElement"`
- UFUNCTIONs:
  - `void InitializeHUDSlot()` — `BlueprintCallable`. Implementation:
    1. Early-out if `bHasSpawnedHUDElement` is true or `SpawnedHUDElement` is already set (idempotent / no double spawn).
    2. Early-out if `HUDElementClass.IsNull()`.
    3. `LoadedClass = HUDElementClass.LoadSynchronous()`; early-out if null. (Synchronous soft-class load — blocking first-use load.)
    4. `SpawnedHUDElement = CreateWidget<UZWUIHUDElement>(GetWorld(), LoadedClass)`.
    5. If created, `AddChildToOverlay(SpawnedHUDElement)` returns a `UOverlaySlot*`; on that slot it calls `SetHorizontalAlignment(SlotHorizontalAlignment)`, `SetVerticalAlignment(SlotVerticalAlignment)`, `SetPadding(SlotPadding)`.
    6. Sets `bHasSpawnedHUDElement = true`.
- UPROPERTYs:
  - `TSoftClassPtr<UZWUIHUDElement> HUDElementClass` — `EditAnywhere`. Soft class reference to the element widget to spawn. (Not `BlueprintReadOnly`; designers set it per slot instance in the panel blueprint.)
  - `TObjectPtr<UZWUIHUDElement> SpawnedHUDElement` — `Transient`. The live spawned element widget instance.
  - `FMargin SlotPadding` — `EditAnywhere`. Padding applied to the overlay slot.
  - `TEnumAsByte<EHorizontalAlignment> SlotHorizontalAlignment` — `EditAnywhere`. Horizontal alignment of the spawned child.
  - `TEnumAsByte<EVerticalAlignment> SlotVerticalAlignment` — `EditAnywhere`. Vertical alignment of the spawned child.
  - `bool bHasSpawnedHUDElement` — `UPROPERTY()` (saved/instance-visible), default `false`. Idempotency guard.

### 4.4 `UZWUIHUDElement` — HUD element widget
- File: `Public/ZWUIHUDElement.h`, `Private/ZWUIHUDElement.cpp`
- Base: `UUserWidget`
- UFUNCTIONs:
  - `void InitializeHUDElement(AActor* OwnerActor)` — `BlueprintCallable`, Category `"HUD Slot"`. **Implementation is an empty body** — subclasses are expected to override/implement binding to the owning actor.
- No UPROPERTYs declared here.

## 5. Implementation (Private)

| File | Contents |
|---|---|
| `ZWUIHUD.cpp` | `FZWUIHUDModule` with empty `StartupModule` / `ShutdownModule`; `LOCTEXT_NAMESPACE "FZWUIHUDModule"`; `IMPLEMENT_MODULE(FZWUIHUDModule, ZWUIHUD)`. No hooks, no console commands, no settings registration. |
| `ZWUIHUDPanel.cpp` | `NativeConstruct()` — as described in §4.2: scans the widget tree for `UZWUIHUDElementSlot` instances and initializes each one. Comments in the implementation are in Polish ("Przeszukujemy każdy pojedynczy widget wrzucony do tego HUDa w edytorze", etc.). |
| `ZWUIHUDElementSlot.cpp` | `InitializeHUDSlot()` — as described in §4.3. Includes `ZWUIHUDElement.h` and `Components/OverlaySlot.h`. |
| `ZWUIHUDElement.cpp` | `InitializeHUDElement(AActor* OwnerActor)` — empty body; no additional includes beyond the header. |

Notes:
- Initialization timing: element spawning happens at panel `NativeConstruct` time, i.e. when the HUD panel widget is constructed — not on activation (`NativeOnActivated` is not overridden here; the inherited one from `UZWUIPanel` runs).
- All begin-play logic is **unrealized on uninit** in the sense that there is no teardown path: slots never reset `SpawnedHUDElement`/`bHasSpawnedHUDElement` on destruction; widget lifetime is managed by UMG garbage collection.
- No logging is active (the only `UE_LOG` is commented out). No error feedback if class load or creation fails — silent early-outs.

## 6. Configuration (.ini)

None. The plugin ships no config files, no `UDeveloperSettings`/`UObject` config classes, and no `.ini` keys are read in code. Configuration is entirely editor-side via UPROPERTYs on `UZWUIHUDElementSlot` (element class, padding, alignment) inside panel blueprints.

## 7. Dependencies within ZWSuite

- **ZWUICore** — the sole in-suite dependency:
  - `UZWUIHUDPanel` extends `UZWUIPanel` (`ZWUICore/Source/ZWUICore/Public/ZWUIPanel.h`, class `ZWUICORE_API`). Inherited surface relevant to HUD panels: `CommonActivatableWidget` base, `GetDesiredInputConfig()`, `PanelIdentityTag` (the tag used to summon the panel, injected by the HUB upon creation), `BoundPanelTag`, `bRequiresBackground`, `bRequiresInput`, lifecycle overrides `NativeOnInitialized`, `NativeOnActivated`, `NativeOnDeactivated`, `NativeGetDesiredFocusTarget`, `NativeOnHandleBackAction`.
  - Header forward-declares `UZWUIRootLayout` and `UZWUIPlayerHUBWidget` (a `UCommonActivatableWidget` subclass in ZWUICore), indicating intended integration with the ZWUICore root-layout/player-HUB activation stack, though ZWUIHUD code itself does not use them.
  - Declared on both levels: `.uplugin` plugin dependency, `Build.cs` public dependency.
- Engine plugin dependencies: **CommonUI** (used transitively via `UZWUIPanel`), **EnhancedInput** (declared in `.uplugin`; no direct use in this plugin's code). Cross-plugin (non-ZWSuite) assets/classes: none.

## 8. Notes / Risks

- **`LoadSynchronous()` on the hot path**: each slot synchronously loads its element class on first panel construction. For many/large elements this blocks the frame during HUD creation; consider async load or `PreLoadScreen` handling if needed.
- **Empty core logic**: `UZWUIHUDElement::InitializeHUDElement()` and both module lifecycle functions are stubs/empty. All real behavior is the slot spawn flow and the `NativeConstruct` scan. Treat the element API as scaffolding to be filled in by subclasses.
- **No teardown/reset**: `SpawnedHUDElement` and `bHasSpawnedHUDElement` are never cleared; if a panel is pooled and re-constructed, the guard only prevents *double* spawn from the same slot instance, but if the slot widget is recreated (e.g., from an expanded widget tree), a new element spawns each time — no per-owner registry exists. Duplicate-slot spawn also triggers only once per slot instance (which is the intended idempotency), but there is no de-duplication across slots referencing the same class.
- **Silent failures**: null world, unloaded class, failed widget creation all bail out without logging (the only log is commented out). Debugging requires enabling the commented log line.
- **Overlapping dependencies**: `CoreUObject`/`Engine` in both public and private dependency lists.
- **Unused forward declarations**: `UZWUIRootLayout`, `UZWUIPlayerHUBWidget` in `ZWUIHUDPanel.h` are not used — likely leftovers of planned integration with the ZWUICore HUB/root layout.
- **Inheritance from `UZWUIPanel` for a HUD**: the HUD panel inherits menu-centric panel semantics (back action, input requirements, background toggle). Defaults (`bRequiresInput = true`, `bRequiresBackground = true`) may be inappropriate for a persistent HUD and must be tuned in subclasses; the input-config behavior of the base may steal input contexts from gameplay.
- **Polish-language comments** in `ZWUIHUDPanel.cpp` — source is reconstructed; harmless, but inconsistent localization in comments.
- Layers cannot currently flow data to elements: the only channel is `InitializeHUDElement(AActor*)`, which nothing in this plugin ever calls — the caller must be game code or a subclass.

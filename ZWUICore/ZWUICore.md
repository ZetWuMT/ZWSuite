# ZWUICore — Technical Documentation

> Source analyzed: `/opt/data/projectx/ZWSuite-src/ZWUICore` (28 files: 1 `.uplugin`, 1 `Build.cs`, 1 `.ini`, 12 public headers, 12 private sources, 1 icon resource).

## 1. Overview

**ZWUICore** is a single-module Unreal Engine plugin that provides a decoupled UI framework built on top of Epic's **CommonUI**. Its description in the `.uplugin` reads: *"Decoupled UI Core framework based on CommonUI."*

The plugin's architecture centers on four cooperating pillars:

1. **`UZWUISubsystem`** — a `ULocalPlayerSubsystem` that owns the whole UI lifecycle: it loads a panel registry (a `UZWUIPanelDatabase` Data Asset) at startup, resolves GameplayTags to panel widget classes, asynchronously (or synchronously) loads those classes, instantiates panels, routes them into the correct layer of the root layout, tracks active panels, and drives the CommonUI **Action Router** input configuration.
2. **`UZWUIRootLayout`** — an abstract `UUserWidget` root layout whose Blueprint must contain three bound containers: `GameLayer` (UOverlay), `MenuLayer` (UCommonActivatableWidgetStack), and `PromptLayer` (UCommonActivatableWidgetStack). This mirrors the layered design of Lyra/CommonUI.
3. **`UZWUIPlayerHUBWidget`** — an abstract, Blueprintable `UCommonActivatableWidget` that acts as the main full-screen menu container (Inventory, Journal, Pause, etc.). It hosts a `UCommonActivatableWidgetSwitcher` of tab panels, an optional `UCommonTabListWidgetBase` tab bar generated from a `UZWUITabConfig` Primary Data Asset, a background widget, and a `UCommonBoundActionBar` footer.
4. **`UZWUIPanel`** — an abstract `UCommonActivatableWidget` that all opened panels derive from. It self-registers with the subsystem on activation/deactivation, binds "generic input actions" from a `UZWInputConfig` (from the sibling **ZWCore** plugin) as hidden CommonUI menu bindings that broadcast GameplayTags, and implements a dual-mode back/close behavior.

A defining design choice is the **dual ownership model**, toggled by `UZWUISettings::bIsUIStateExternallyManaged`:

- **Standalone mode (default)** — panels and the HUB close themselves through native CommonUI `DeactivateWidget()` flows, and a synthesized `UI.State.Back` GameplayTag is broadcast on `OnGameplayTagSent` when a panel is unregistered or the HUB is deactivated.
- **Externally managed mode** (e.g., driven by an external State Tree manager, referenced in comments as "ZWUIStateTree") — widgets never close themselves. Instead they broadcast a configured `ExternalCloseTag` (panels) or per-tab `StateTag` (HUB tab switching), and one-off deactivation events are silenced; the external manager is responsible for actually removing panels.

Panel routing is fully tag-driven via four **native GameplayTags** defined in `ZWUIGameplayTags`:

| Native tag | Reached path |
|---|---|
| `UI.Panel.HUD` | Not routed by `RequestPanelWidget` (HUD is created directly from settings in `PlayerControllerChanged`) |
| `UI.Panel.Prompt` | Added to `RootLayout->PromptLayer` |
| `UI.Panel.Menu.Standalone` | Added to `RootLayout->MenuLayer` |
| `UI.Panel.Menu.Tab` | Opened inside the Player HUB's `MenuPanelsSwitcher` (HUB is spawned on demand into `MenuLayer`) |

## 2. Plugin Metadata (`.uplugin`)

`ZWUICore.uplugin`:

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | `ZWUICore` |
| Description | "Decoupled UI Core framework based on CommonUI." |
| Category | `UI` |
| CreatedBy | `tiramisoo` |
| CreatedByURL | *(empty)* |
| DocsURL | *(empty)* |
| MarketplaceURL | *(empty)* |
| EnabledByDefault | false |
| CanContainContent | true |
| IsBetaVersion | false |
| IsExperimentalVersion | false |
| Installed | false |

**Modules** (1 entry):

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWUICore` | Runtime | Default |

**Plugin dependencies** (declared in `.uplugin` `Plugins` array):

| Plugin | Enabled |
|---|---|
| `ZWCore` | true |
| `CommonUI` | true |
| `EnhancedInput` | true |

## 3. Module (Build.cs)

Single module: `Source/ZWUICore/ZWUICore.Build.cs`, class `ZWUICore : ModuleRules`.

- `PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs`
- `PublicIncludePaths`: *(empty)*
- `PrivateIncludePaths`: *(empty)*
- `PublicDependencyModuleNames`: `Core`, `GameplayTags`, `CommonUI`, `DeveloperSettings`, `InputCore`
- `PrivateDependencyModuleNames`: `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `UMG`, `EnhancedInput`, `ZWCore`
- `DynamicallyLoadedModuleNames`: *(empty, comment placeholder only)*

Note: `ZWCore` (the sibling plugin providing `UZWInputConfig` / `FZWInputAction` used by `UZWUIPanel` and `UZWUISettings`) is only a **private** dependency despite being referenced from public headers via forward declarations and `LoadSynchronous` usage — headers only forward-declare `UZWInputConfig`, so this is consistent, but any public header including `ZWInputConfig.h` would leak the dependency.

## 4. Public API — Classes

Ten C++ classes are exported (`ZWUICORE_API`) plus three USTRUCTs and one enum. All widget/data classes live in module `ZWUICore`.

### 4.1 `FZWUICoreModule` (`Public/ZWUICore.h`, `Private/ZWUICore.cpp`)

Module entry point implementing `IModuleInterface`.
- `virtual void StartupModule() override` — empty (comment only).
- `virtual void ShutdownModule() override` — empty.
- `IMPLEMENT_MODULE(FZWUICoreModule, ZWUICore)`; LOCTEXT namespace `FZWUICoreModule`.

### 4.2 `UZWUISubsystem` (`Public/ZWUISubsystem.h`) — `ULocalPlayerSubsystem`

The central orchestrator.

**Delegate**
- `DECLARE_MULTICAST_DELEGATE_OneParam(FOnUITagSentDelegate, FGameplayTag)`
- `FOnUITagSentDelegate OnGameplayTagSent;` — *commented-out BlueprintAssignable*; C++-only broadcast bus used for back/close notification, tab state changes, and generic input actions. Every consumer binds via `AddUObject`.

**Struct `FZWActivePanelContext` (USTRUCT)**
- `UPROPERTY() UZWUIPanel* Panel = nullptr;`
- `UPROPERTY() FGameplayTag PanelTag;`
- `bool operator==(const UZWUIPanel* OtherPanel) const` — enables `IndexOfByKey(Panel)` lookups by raw pointer.

**Public interface**
| Member | Signature | Purpose |
|---|---|---|
| `Initialize` | `void Initialize(FSubsystemCollectionBase&) override` | Load `UZWUISettings::PanelRegistry` synchronously; registers every `FZWUIPanelData` entry; editor-only debug logging per panel; warns if registry fails to load. |
| `PlayerControllerChanged` | `void PlayerControllerChanged(APlayerController* NewPlayerController) override` | Stores the PC; creates `MainRootLayoutClass` via `CreateWidget`, `AddToViewport(0)`; loads `MainHUDClass` synchronously and adds it as a fill/fill `UOverlaySlot` into `RootLayout->GameLayer`. Warns if layout creation fails. |
| `RegisterPanelData` | `UFUNCTION(BlueprintCallable, Category="ZW\|UI") void RegisterPanelData(const FZWUIPanelData& PanelData)` | Adds mapping into `RegisteredPanels` if tag valid and class non-null. |
| `RequestPanelWidget` | `UFUNCTION(BlueprintCallable, Category="ZW\|UI\|Input") void RequestPanelWidget(FGameplayTag PanelTag)` | Main entry point for opening panels. Early-outs if no PC/layout/invalid tag; de-nulls stale `PlayerHUB`; fast-path for already-instanced `Panel_Menu_Tab` panels when HUB alive; resolves class, queues HUB class load when needed; rejects tags not matching `Panel_Menu_Tab` / `Panel_Menu_Standalone` / `Panel_Prompt` (warns via `LogTemp`); dedupes in-flight loads via `ActiveLoadHandles`; async-loads via `UAssetManager::GetStreamableManager()`. |
| `GetOrCreateInstancedPanel` | `UFUNCTION(BlueprintCallable, Category="ZW\|UI") UZWUIPanel* GetOrCreateInstancedPanel(FGameplayTag PanelTag)` | Returns cached panel from `InstancedPanels`, or synchronously loads the class, creates the widget, sets both `BoundPanelTag` and `PanelIdentityTag`, caches it, and returns it. Sole owner of panel instances. |
| `ClosePanelWidget` | `UFUNCTION(BlueprintCallable, Category="ZW\|UI") void ClosePanelWidget(FGameplayTag PanelTag)` | If given tag matches `MainHUBTag` and HUB active — deactivates HUB and nulls it. Else deactivates the cached instanced panel; fallback scans `ActivePanels` backwards for exact `BoundPanelTag` match. Warns when nothing found. |
| `RegisterPanel` | `void RegisterPanel(UZWUIPanel* Panel)` | Appends `FZWActivePanelContext` (dedup by pointer) with `Panel->BoundPanelTag`, then `RefreshInputConfig()`. |
| `UnregisterPanel` | `void UnregisterPanel(UZWUIPanel* Panel)` | Removes context, refreshes input; if `bIsUIStateExternallyManaged` — stays silent; otherwise, for non-`Panel_Menu_Tab` panels, broadcasts `UI.State.Back` on `OnGameplayTagSent`. |
| `IsPanelRegisteredByTag` | `bool IsPanelRegisteredByTag(FGameplayTag UITag)` | Predicate search over `ActivePanels` — **exact tag equality** is live code (hierarchical `MatchesTag` variant is present but commented out). |

**Protected/private internals**
- `RefreshInputConfig()` — if panels active: takes the top panel's `GetDesiredInputConfig()`, falling back to `FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture)`; otherwise game config `ECommonInputMode::Game` + `EMouseCaptureMode::CapturePermanently` with look/move input not ignored. Pushes via `Router->SetActiveUIInputConfig(...)`.
- `GetActionRouter() const` — `UCommonUIActionRouterBase*` from the first game player's LocalPlayer subsystem.
- `GetPanelClass(FGameplayTag) const` — returns `TSoftClassPtr<UZWUIPanel>` from `RegisteredPanels` (nullptr if absent/null).
- `AddPanelToLayer(FGameplayTag, TSubclassOf<UZWUIPanel>) -> bool` — tag routing: `Panel_Menu_Tab` → reuse/create instanced panel, `PlayerHUB->OpenTabInSwitcher(Panel)`; `Panel_Menu_Standalone` → `RootLayout->MenuLayer->AddWidget` with `BoundPanelTag` init lambda; `Panel_Prompt` → same into `PromptLayer`.
- `OnPanelWidgetClosed()` — placeholder ("possibly handle input refresh").
- `OnPanelClassLoaded(FGameplayTag, TSoftClassPtr<UZWUIPanel>)` — clears the load handle; if `Panel_Menu_Tab` and no HUB, spawns `MainHUBClass` into `RootLayout->MenuLayer`; then `AddPanelToLayer`.

**State**
| Field | Type |
|---|---|
| `InstancedPanels` | `UPROPERTY() TMap<FGameplayTag, UZWUIPanel*>` |
| `ActiveLoadHandles` | `TMap<FGameplayTag, TSharedPtr<FStreamableHandle>>` (not UPROPERTY — raw TSharedPtr) |
| `RegisteredPanels` | `UPROPERTY() TMap<FGameplayTag, FZWUIPanelData>` |
| `ActivePanels` | `UPROPERTY() TArray<FZWActivePanelContext>` |
| `RootLayout` | `UPROPERTY() UZWUIRootLayout*` |
| `HUD` | `UPROPERTY() UZWUIPanel*` |
| `PlayerHUB` | `UPROPERTY() UZWUIPlayerHUBWidget*` |
| `PromptStack` | `UPROPERTY() UCommonActivatableWidgetStack*` |
| `PlayerController` | `UPROPERTY() APlayerController*` |

### 4.3 `UZWUIPanel` (`Public/ZWUIPanel.h`) — `UCommonActivatableWidget`, `UCLASS(Abstract)`

Base class for every openable panel.

| Member | Details |
|---|---|
| Constructor | Sets `bIsModal = true` and `bIsBackHandler = true` — panels are modal and handle back input by default. |
| `PanelIdentityTag` | `UPROPERTY BlueprintReadOnly, Category="ZW\|UI"` `FGameplayTag` — the tag used to summon the panel, injected on creation. |
| `bRequiresBackground` | `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)` `bool = true` — read by the HUB to show/hide `MenuBackground`. |
| `BoundPanelTag` | `UPROPERTY(Transient)` `FGameplayTag` — bound runtime tag used for closing/unregistration. |
| `bRequiresInput` | `UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)` `bool = true` — gates Register/Unregister panel on activation. |
| `GetDesiredInputConfig()` | `override` `TOptional<FUIInputConfig>` — currently defers to Super with a TODO for custom configs. |
| `NativeOnInitialized()` | `override` — synchronously loads `UZWUISettings::InputConfig` (`UZWInputConfig`, from ZWCore); for each `FZWInputAction` in `GenericInputActions` binds the `UInputAction` via `FBindUIActionArgs` with `InputMode = ECommonInputMode::Menu` and `bDisplayInActionBar = false`; the handler broadcasts the action's `InputTag` on `Subsystem->OnGameplayTagSent` (weak-lambda bound). |
| `NativeOnActivated()` | `override` — if `bRequiresInput`, calls `Subsystem->RegisterPanel(this)`. |
| `NativeOnDeactivated()` | `override` — if `bRequiresInput`, calls `Subsystem->UnregisterPanel(this)`. |
| `NativeGetDesiredFocusTarget()` | `override` — defers to CommonUI. |
| `NativeOnHandleBackAction()` | `override` — **dual mode**: if `bIsUIStateExternallyManaged`, broadcasts `Settings->ExternalCloseTag` and returns `true` without closing (external manager closes later); otherwise `Super::NativeOnHandleBackAction()` performs the native `DeactivateWidget()`. |

### 4.4 `UZWUIRootLayout` (`Public/ZWUIRootLayout.h`) — `UUserWidget`, `UCLASS(Abstract)`

Root viewport layout; `.cpp` is a bare include (all logic in the derived Blueprint and in widgets referencing its members).

| Member | Details |
|---|---|
| `GameLayer` | `UPROPERTY(BlueprintReadOnly, meta=(BindWidget))` `UOverlay*` — in-game HUD layer (HUD widget inserted here by the subsystem). |
| `MenuLayer` | `UPROPERTY(... BindWidget)` `UCommonActivatableWidgetStack*` — hosts the Player HUB and standalone menu panels. |
| `PromptLayer` | `UPROPERTY(... BindWidget)` `UCommonActivatableWidgetStack*` — hosts prompt panels. |

### 4.5 `UZWUIPlayerHUBWidget` (`Public/ZWUIPlayerHUBWidget.h`) — `UCommonActivatableWidget`, `UCLASS(Abstract, Blueprintable)`

*"The main container for all UI panels. Listens to the UI Subsystem and dynamically loads/switches panels using a Switcher."*

| Member | Details |
|---|---|
| `SetMenuBackgroundVisible(bool)` | `UFUNCTION(BlueprintCallable, Category="UI\|Visuals")` — Visible/Collapsed on `MenuBackground`. |
| `OpenTabInSwitcher(UZWUIPanel*)` | C++ — adds panel as child of `MenuPanelsSwitcher` if missing; with `MenuTabList` present: `SelectTabByID(TabID)` (CommonUI handles reactivation/registration itself); without: `SetActiveWidget` + manual `ActivateWidget()`. Then `SetMenuBackgroundVisible(Panel->bRequiresBackground)` and (re)binds `OnDeactivated` → `OnTabClosed` using `RemoveAll(this)` first to avoid double-binding. |
| `OnTabClosed()` | C++ — silenced when `bIsUIStateExternallyManaged` or the switcher is mid-switch; if the active panel is null or deactivated: hides background and deactivates the HUB itself; otherwise re-evaluates `bRequiresBackground` of the new active panel. |
| `GetInstancedTab(FGameplayTag)` | `UFUNCTION(BlueprintPure, Category="ZW\|UI") UZWUIPanel*` — lookup into `InstancedTabs`. |
| `NativeConstruct()` | `override` — resolves `UZWUISubsystem` from owning local player; if `MenuTabList && MenuPanelsSwitcher && TabConfiguration && Subsystem`: links switcher (`SetLinkedSwitcher`), binds `OnTabSelected` (`AddUniqueDynamic` → `HandleTabSelected`), then for every `FZWUITabDefinition`: skips existing tab buttons, asks the subsystem `GetOrCreateInstancedPanel` (instance ownership stays with the subsystem), adds to switcher, `RegisterTab(TabID, TabButtonClass, Panel)`, casts the created button to `UZWUITabListButton` and pushes `SetTabData(TabDisplayName, TabIcon)`; binds `OnDeactivated` → `OnTabClosed` (RemoveAll + AddUObject). |
| `NativeDestruct()` | `override` — cancels all active `FStreamableHandle`s in `ActiveLoadHandles` to avoid crashes/leaks, empties the map. |
| `NativeOnDeactivated()` | `override` — externally-managed: silent; standalone: broadcasts `UI.State.Back` through `Subsystem->OnGameplayTagSent`. |
| `HandleTabSelected(FName TabId)` | `UFUNCTION()` — guards against null config and mid-switch; resolves the matching `FZWUITabDefinition` → its `StateTag`; externally-managed: silent; otherwise broadcasts the tab's `StateTag` on `OnGameplayTagSent` (with a debug `LogTemp` line). |
| `OnWidgetClassLoaded(FGameplayTag, TSoftClassPtr<UZWUIPanel>)` | private — clears handle; creates tab widget, sets `PanelIdentityTag`, caches into `InstancedTabs`, adds to switcher, activates it, applies background visibility, binds `OnTabClosed`. *(Path is currently unreachable from the hot flow — the HUB now instantiates tabs via the subsystem.)* |

**Bound widgets** (required unless noted): `MenuBackground` (`UWidget*`), `MenuPanelsSwitcher` (`UCommonActivatableWidgetSwitcher*`), `MenuPanelsOverlay` (`UOverlay*`), `FooterActionBar` (`UCommonBoundActionBar*`, `BlueprintReadOnly`), and **optional** `MenuTabList` (`UCommonTabListWidgetBase*`, `BindWidgetOptional`). Config asset: `TabConfiguration` (`TObjectPtr<UZWUITabConfig>`, `EditDefaultsOnly, Category="ZW\|Tabs"`) — optional; if unset the generated-tab-button path in `NativeConstruct` is skipped entirely.

### 4.6 `UZWUIPanelDatabase` (`Public/ZWUIPanelDatabase.h`) — `UDataAsset`

Simple panel registry consumed at subsystem `Initialize` (via `UZWUISettings::PanelRegistry`).

Also defines the shared enum and struct in the same header:

- `UENUM(BlueprintType) enum class EZWWidgetLayer : uint8 { GameLayer, MenuLayer, PromptLayer }` — declared logically in this header but forward-declared broadly elsewhere; note the subsystem/HUB forward declarations of it are largely unused in the current code paths.
- `USTRUCT(BlueprintType) struct FZWUIPanelData { FGameplayTag PanelTag; TSoftClassPtr<UZWUIPanel> PanelClass; }` — both `EditAnywhere, BlueprintReadOnly`.
- `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="ZW UI") TArray<FZWUIPanelData> Panels;`

### 4.7 `UZWUITabConfig` (`Public/ZWUITabConfig.h`) — `UPrimaryDataAsset`, `UCLASS(BlueprintType)`

Drives the HUB's tab bar generation.

- `USTRUCT(BlueprintType) FZWUITabDefinition`
  - `TabTag` — `FGameplayTag`, unique tab id (e.g. "UI.Panel.Inventory"); becomes the CommonUI `FName` TabID.
  - `StateTag` — `FGameplayTag` (category "Tab|State Tree") — broadcast when the tab is selected and the UI is externally managed.
  - `PanelClass` — `TSubclassOf<UCommonActivatableWidget>` — the widget opened in the switcher.
  - `TabButtonClass` — `TSubclassOf<UCommonButtonBase>` — button look for the top bar.
  - `TabIcon` — `UTexture2D*` (optional).
  - `TabDisplayName` — `FText`.
- `UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Tabs") TArray<FZWUITabDefinition> Tabs;`

⚠️ Note: `FZWUITabDefinition::PanelClass` is `TSubclassOf<UCommonActivatableWidget>` while the HUB actually instantiates panels through `UZWUISubsystem::GetOrCreateInstancedPanel(TabTag)` (which uses the *registry's* `TSoftClassPtr<UZWUIPanel>`); the struct's class field is effectively unused by the current HUB flow.

### 4.8 `UZWUITabList` (`Public/ZWUITabList.h`) — `UCommonTabListWidgetBase`, `UCLASS(Abstract, BlueprintType)`

Tab-list wrapper that places generated buttons into a custom container.

- `TabButtonContainer` — `UPROPERTY(BindWidget)` `UPanelWidget*`.
- `HandleTabCreation_Implementation(FName, UCommonButtonBase*)` — adds generated button to `TabButtonContainer`.
- `HandleTabRemoval_Implementation(FName, UCommonButtonBase*)` — removes it.

### 4.9 `UZWUITabListButton` (`Public/ZWUITabListButton.h`) — `UCommonButtonBase`, `UCLASS(Abstract, BlueprintType)`

Tab button with optional icon/text widgets.

- `SetTabData(FText ButtonText, UTexture2D* ButtonIcon)` — `UFUNCTION(BlueprintCallable)`; sets `TabButtonText` text; if icon and `TabButtonImage` present, `SetBrushFromSoftTexture(ButtonIcon)`.
- `TabButtonImage` — `UPROPERTY(BindWidgetOptional)` `UImage*`.
- `TabButtonText` — `UPROPERTY(BindWidgetOptional)` `UCommonTextBlock*`.

### 4.10 `UZWUISettings` (`Public/ZWUISettings.h`) — `UDeveloperSettings`, `UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW UI Core"))`

Project-settings config object; appears under the editor category `ZW`, section *"ZW UI Core Settings"* (editor-only `GetCategoryName` / `GetSectionText` overrides).

| Property | Type / default | Purpose |
|---|---|---|
| `MainRootLayoutClass` | `TSubclassOf<UUserWidget>` (Config) | Root layout spawned into viewport on `PlayerControllerChanged`. |
| `MainHUDClass` | `TSoftClassPtr<UZWUIPanel>` (Config) | In-game HUD added into `RootLayout->GameLayer`. |
| `MainHUBClass` | `TSoftClassPtr<UZWUIPlayerHUBWidget>` (Config) | Fullscreen HUB (Inventory, Progression, Pause). Doc comment stresses it is NOT the game HUD — that one is specified in GameMode. |
| `MainHUBTag` | `FGameplayTag` (Config; EditCondition `bIsUIStateExternallyManaged`, hides) | Tag identifying the HUB for `ClosePanelWidget`. |
| `PanelRegistry` | `TSoftObjectPtr<UZWUIPanelDatabase>` (Config, ForceInlineRow) | Default panel database loaded at subsystem init. |
| `PromptPanels` | `TMap<FGameplayTag, TSoftClassPtr<UUserWidget>>` (Config, ForceInlineRow) | Prompt panel mapping (declared; not consumed by any current code path). |
| `InputConfig` | `TSoftObjectPtr<UZWInputConfig>` (Config, ForceInlineRow) | ZWCore input config consumed by `UZWUIPanel::NativeOnInitialized`. |
| `bIsUIStateExternallyManaged` | `bool = false` (Config) | true ⇒ panels do not close themselves after the back/exit action; an external manager (e.g. ZWUIStateTree) receives the broadcast tag and removes the panel via a "Close Panel" task. |
| `ExternalCloseTag` | `FGameplayTag` (Config; EditCondition as above, hides) | Tag broadcast on back action when externally managed. |
| `MenuPanels` | private `UPROPERTY() TMap<FGameplayTag, TSoftClassPtr<UZWUIPanel>>` | Private, currently unfilled/unread by any code path found. |

Constructor `UZWUISettings::UZWUISettings()` is empty. `.cpp` matches the header's 9 member functions/overrides (constructor, Initialize/PlayerControllerChanged live in subsystem, GetCategoryName, GetSectionText — total declared surface in header is 1 constructor + 9 properties + 2 editor-only overrides; `.cpp` only defines the constructor plus nothing else beyond the include).

## 5. Implementation Notes (Private)

| File | Contents |
|---|---|
| `ZWUICore.cpp` | `IMPLEMENT_MODULE` boilerplate; empty startup/shutdown; LOCTEXT namespace `FZWUICoreModule`. |
| `ZWUISubsystem.cpp` | All orchestration logic described in §4.2 (registry preload, layout/HUD boot, tag routing, async load machinery, close/unregister with back-tag fallback, input config refresh, Action Router access through first game player). |
| `ZWUIPanel.cpp` | Modal/back-handler defaults, generic-input binding from `UZWInputConfig`, register/unregister on activation, dual-mode back handling. |
| `ZWUIPlayerHUBWidget.cpp` | Tab lifecycle: HUB spawn negotiation stays in the subsystem, HUB only requests instances; manual switcher path when no tab list; pending-load cancellation on destruct; back-tag/state-tag broadcasts gated by `bIsUIStateExternallyManaged`. |
| `ZWUIRootLayout.cpp` | Only an include — intentionally empty (layout is Blueprint-driven via BindWidget). |
| `ZWUISettings.cpp` | Empty constructor only. |
| `ZWUIPanelDatabase.cpp` | Only an include. |
| `ZWUITabConfig.cpp` | Only an include. |
| `ZWUITabList.cpp` | Creation/removal handlers moving buttons into `TabButtonContainer`. |
| `ZWUITabListButton.cpp` | `SetTabData` implementation. |
| `ZWUILogChannels.cpp` | `DEFINE_LOG_CATEGORY(LogZWUICore);` |
| `ZWUIGameplayTags.cpp` | `UE_DEFINE_GAMEPLAY_TAG` for `Panel_HUD = "UI.Panel.HUD"`, `Panel_Prompt = "UI.Panel.Prompt"`, `Panel_Menu_Standalone = "UI.Panel.Menu.Standalone"`, `Panel_Menu_Tab = "UI.Panel.Menu.Tab"`. |

## 6. Configuration (`Config/DefaultZWUICore.ini`)

The plugin ships a single config file containing one **CoreRedirect** (asset-property rename support):

```ini
[CoreRedirects]
+PropertyRedirects=(OldName="/Script/ZWUICore.ZWUIPanel.BackInputAction",NewName="/Script/ZWUICore.ZWUIPanel.AdditionalBackInputAction")
```

This redirects the old `UZWUIPanel::BackInputAction` property to `AdditionalBackInputAction` for existing serialized assets. Notably, neither `BackInputAction` nor `AdditionalBackInputAction` exists in the current `UZWUIPanel` header — the redirect is a leftover (or preparation) for a property that was removed/never landed; see §8.

All other runtime configuration lives in `UZWUISettings` (Config=Game / defaultconfig, serialized into the game's `DefaultGame.ini` by the editor), not in this plugin's ini.

## 7. Dependencies Within ZWSuite

- **ZWCore** (plugin dependency, enabled in `.uplugin`; private Build.cs dependency):
  - `UZWInputConfig` — forward-declared in `ZWUISettings.h` (`InputConfig` property) and included/loaded in `ZWUIPanel.cpp` (`ZWInputConfig.h`).
  - `FZWInputAction` struct — per-action `InputTag` + soft `InputAction` pairs consumed in `UZWUIPanel::NativeOnInitialized` (`LoadedConfig->GenericInputActions`).
  - Comments in `ZWUIPanel.cpp` reference parity with `ZWInputComponent`'s broadcast behavior.
- **ZWUIStateTree** — not a compile dependency, but the architecture reserves the externally-managed mode (`bIsUIStateExternallyManaged`, `ExternalCloseTag`, per-tab `StateTag`, ` MainHUBTag`) for an external state-machine manager; `HandleTabSelected` and header comments describe a 'Close Panel' State Tree task. No header of it is included here.
- No other ZWSuite plugin (e.g., ZWGameplayUI consumers) is referenced from ZWUICore sources.

## 8. Notes / Risks

1. **Editable `InputConfig.LoadSynchronous()` on panel init** — `UZWUIPanel::NativeOnInitialized` performs a synchronous asset load per panel; acceptable at menu load time, but hot-opening many panels may hitch.
2. **`RequestPanelWidget` gap for externally-managed back flow with no HUB** — the HUB class is loaded only when `Panel_Menu_Tab` requests arrive and HUB is missing; the close path for `MainHUBTag` requires `MainHUBTag` to be valid, which the settings UI hides unless `bIsUIStateExternallyManaged` is on — an odd coupling (the hide condition is on external management, yet `ClosePanelWidget` uses the tag in both modes).
3. **`ActiveLoadHandles` is a raw `TSharedPtr<FStreamableHandle>` map without UPROPERTY** in the subsystem (GC-safe since handles aren't UObject-referencing, but the HUB's identical map is also non-UPROPERTY while its keys include soft class loads — softened by `NativeDestruct` cancellation).
4. **Dead/leftover code paths**: `UZWUIPlayerHUBWidget::OnWidgetClassLoaded` has no current caller in its own class flow (superseded by the subsystem's `GetOrCreateInstancedPanel` path); `UZWUISubsystem::OnPanelWidgetClosed` is a stub; `FZWActivePanelContext`-style comments and `UZWUIPanel::HandleBackAction` exist only in commented-out form; the `Config/DefaultZWUICore.ini` redirect targets a property absent from the header.
5. **`UZWUISettings::PromptPanels`** (tag → widget-class map) has no consumer in ZWUICore code — prompts are routed via the registry instead; either dead config surface or an integration point for another module.
6. **`UZWUITabDefinition::PanelClass`** is not used by the HUB flow (instances come from the registry by `TabTag`); `TabIcon`/`TabDisplayName` are only applied when the button casts to `UZWUITabListButton`.
7. **`IsPanelRegisteredByTag` uses exact match** — a commented hierarchical (`MatchesTag`) variant exists; callers expecting child-tag matching will not find it.
8. **`LogTemp` used in some paths** (`RequestPanelWidget` rejection, `HandleTabSelected`) instead of `LogZWUICore` — log filtering by category will miss these messages.
9. **`UZWUISubsystem::HUD`, `PromptStack`, `EZWWidgetLayer`** are declared but never assigned/read in the current implementation — likely scaffolding for prompt stacks / layer metadata.
10. **Mixed-language comments** (Polish + English) throughout the reconstructed sources; behavior descriptions here were cross-checked against actual code paths, with uncertain-comment cases labeled as such above.
11. **Public API surface is minimal but tag-coupled**: any consumer must author matching GameplayTags in an INI tag table (`UI.Panel.*`, `UI.State.Back`) for the routing/close fallback to function.

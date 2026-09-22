# ZWInput — Technical Documentation

## 1. Overview

ZWInput is a small, runtime-only Unreal Engine plugin inside the ZWSuite family that provides a **decoupled input framework on top of EnhancedInput**. Instead of binding code directly to `UInputAction` assets, it introduces a tag-driven indirection layer:

- A project-level configuration asset (`UZWInputConfig`, defined in the **ZWCore** plugin) maps `FGameplayTag` input tags to `UInputAction` assets and trigger events.
- A `UDeveloperSettings` subclass (`UZWInputSettings`) points to that config asset via a soft object pointer, stored in the game config.
- A custom `UZWInputComponent` (subclass of `UEnhancedInputComponent`) reads the config, binds all "_generic" actions to an internal handler, and rebroadcasts inputs as **GameplayTag-based multicast events** (`OnInputTagTriggered`, `OnInputTagSimpleTriggered`). Consumers therefore react to input tags rather than concrete input assets, fully decoupling gameplay code from input data.
- A template helper `BindNativeAction<>()` allows direct, classic binding of an action for a given tag (bypassing the tag-broadcast path when a class needs the raw callback).

The plugin is minimal: 1 module, 3 game-facing classes (module class, settings, component), no editor-only code beyond editor settings metadata, and no .ini files of its own (`Config=Game` settings are written to the project's `DefaultGame.ini` by the editor, not shipped in the plugin).

## 2. Metadata (.uplugin)

Source: `ZWInput.uplugin`

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | `1.0` |
| FriendlyName | `ZWInput` |
| Description | `Decoupled input framework based on EnhancedInput.` |
| Category | `Input` |
| CreatedBy | `tiramisoo` |
| CreatedByURL | (empty) |
| DocsURL | (empty) |
| MarketplaceURL | (empty) |
| EnabledByDefault | false |
| CanContainContent | true |
| IsBetaVersion | false |
| IsExperimentalVersion | false |
| Installed | false |

**Modules** (1 total):

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWInput` | Runtime | Default |

**Plugin dependencies** (declared in `.uplugin`, both must be enabled):

| Name | Enabled |
|---|---|
| `ZWCore` | true |
| `EnhancedInput` | true |

Resource assets: `Resources/Icon128.png` (plugin icon).

## 3. Modules (Build.cs)

Single build module: `Source/ZWInput/ZWInput.Build.cs`, class `ZWInput : ModuleRules`.

- `PCHUsage = UseExplicitOrSharedPCHs`
- PublicIncludePaths / PrivateIncludePaths: none (template placeholders only)
- **PublicDependencyModuleNames**: `Core`, `GameplayTags`, `EnhancedInput`, `DeveloperSettings`, `ZWCore`
- **PrivateDependencyModuleNames**: `CoreUObject`, `Engine`, `Slate`, `SlateCore`
- DynamicallyLoadedModuleNames: none

Notes:
- `GameplayTags` is a public dependency because `FGameplayTag` appears in public headers (bind/delegate APIs).
- `DeveloperSettings` is public because `UZWInputSettings : UDeveloperSettings` is exported (`ZWINPUT_API`).
- `ZWCore` is a hard public dependency — the component includes `ZWInputConfig.h`, which lives in ZWCore (see §7). The plugin therefore cannot compile or load without ZWCore.
- `EnhancedInput` base classes are referenced in public headers, hence public linkage.

## 4. Publiczny API — klasy

The plugin exposes 3 exported UCLASSes/module class across 3 public headers.

### 4.1 `FZWInputModule` (`Public/ZWInput.h`)

- **Base**: `IModuleInterface`
- Signature: `class FZWInputModule : public IModuleInterface` (plain C++ class, not UCLASS; registered via `IMPLEMENT_MODULE(FZWInputModule, ZWInput)`).
- Methods:
  - `virtual void StartupModule() override;` — empty implementation.
  - `virtual void ShutdownModule() override;` — empty implementation.
- Purpose: standard module skeleton; no runtime initialization is performed at module load time. All functionality is component/config driven.

### 4.2 `UZWInputSettings` (`Public/ZWInputSettings.h`)

- **Base**: `UDeveloperSettings` (`#include "Engine/DeveloperSettings.h"`)
- `ZWINPUT_API` (importable from other modules); `UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Input"))`
- Declarations only: defines no UPROPERTYs of its own beyond the config reference listed below; the `.cpp` is a bare include (no implementation code).
- Forward-declared referenced types (defined elsewhere): `UZWInputConfig` (ZWCore), `UStateTree` (Engine, currently unused in the header body), `UZWInputConfig_Old` (legacy config type, currently unused).
- UPROPERTYs:
  - `UPROPERTY(Config, EditAnywhere, Category="Input", meta=(ForceInlineRow)) TSoftObjectPtr<UZWInputConfig> InputConfig;`
    - Soft reference to the input mapping asset; serialized into the project's `Game` config section for this class so all `UZWInputComponent`s share the same mapping.
- Methods (editor-only overrides, `WITH_EDITORONLY_DATA`):
  - `virtual FName GetCategoryName() const override { return FName("ZW"); }` — places the settings page under the "ZW" category in Project Settings.
  - `virtual FText GetSectionText() const override { return INVTEXT("ZW Input Settings"); }` — localized-invariant section label.
- Purpose: single project-wide editor-settings entry point ("Project Settings → ZW → ZW Input Settings") that selects which `UZWInputConfig` asset the input component consults.

### 4.3 `UZWInputComponent` (`Public/ZWInputComponent.h`)

- **Base**: `UEnhancedInputComponent`
- `ZWINPUT_API`; `UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))` — spawnable/attachable from Blueprints in the editor.
- Delegates (module-level types):
  - `DECLARE_MULTICAST_DELEGATE_TwoParams(FZWInputTagEvent, FGameplayTag /*InputTag*/, const FInputActionValue& /*InputActionValue*/);`
  - `DECLARE_MULTICAST_DELEGATE_OneParam(FZWInputTagSimpleEvent, FGameplayTag /*InputTag*/);`
- Public UPROPERTYs/members:
  - `FZWInputTagEvent OnInputTagTriggered;` — broadcast with (InputTag, InputActionValue) for every generic action.
  - `FZWInputTagSimpleEvent OnInputTagSimpleTriggered;` — broadcast with (InputTag) only; convenience path for handlers that ignore the value.
- Public methods:
  - `UZWInputComponent();` — constructor; `PrimaryComponentTick.bCanEverTick = false` (no tick).
  - `void InitializeInput();` — resolves the config asset from settings, then for each `FZWInputAction` in `CachedConfig->GenericInputActions` with a valid `InputTag`, synchronously loads the `UInputAction` soft reference and calls the inherited `BindAction(InputAction, GenericInputAction.TriggerEvent, this, &UZWInputComponent::HandleGenericInput, ActionTag)`. The tag is forwarded as the extra payload argument to the handler.
  - `template<class UserClass, typename FuncType> void BindNativeAction(FGameplayTag InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);` — **inline template defined in the header** (body visible in the .h). Behavior: lazily fills `CachedConfig` via `GetInputConfigAsset()`; returns silently if no config; iterates `CachedConfig->NativeInputActions`, matches entries whose `InputTag` `MatchesTagExact(InputTag)`, synchronously loads the `UInputAction` and calls inherited `BindAction(IA, TriggerEvent, Object, Func)` to bind directly to the caller's member function (classic LYRA-style native binding,the direct path vs. the broadcast path).
- Private members:
  - `void HandleGenericInput(const FInputActionValue& ActionValue, FGameplayTag InputTag);` — broadcast handler; fires both `OnInputTagTriggered.Broadcast(InputTag, ActionValue)` and then `OnInputTagSimpleTriggered.Broadcast(InputTag)`.
  - `const UZWInputConfig* GetInputConfigAsset();` — reads `GetDefault<UZWInputSettings>()`, synchronously loads `InputSettings->InputConfig` (soft ref). Returns `nullptr` and logs `UE_LOG(LogTemp, Warning, TEXT("ZWInputComponent: UIInputConfig is missing!"))` when the config asset is missing; logs `TEXT("ZWInputComponent: UISettings are missing!")` if the settings CDO is null (effectively unreachable in practice).
  - `UPROPERTY() const UZWInputConfig* CachedConfig = nullptr;` — hard UPROPERTY reference to the loaded config, keeping the asset (and its chain) from being GC'd once loaded.

Key API characteristics:
- All input action assets are loaded with **`LoadSynchronous()`** — no async streaming.
- Loose vs. exact tags: generic broadcast path accepts any valid tag; native binding requires `MatchesTagExact`.
- The component must have `InitializeInput()` called by the owner (no automatic binding in constructor / `BeginPlay` override — the .cpp contains only the constructor and the three methods above).

## 5. Implementation (Private)

`Source/ZWInput/Private/`:

- `ZWInput.cpp` — boilerplate: `LOCTEXT_NAMESPACE "FZWInputModule"`, empty Startup/Shutdown, `IMPLEMENT_MODULE(FZWInputModule, ZWInput)`.
- `ZWInputSettings.cpp` — only `#include "ZWInputSettings.h"`; no additional code (UCLASS reflection handles the property & editor-only virtual overrides).
- `ZWInputComponent.cpp`:
  - Includes: `ZWInputComponent.h`, `GameplayTagContainer.h`, `ZWInputSettings.h`.
  - Constructor: sets `PrimaryComponentTick.bCanEverTick = false` (comment template text retained from Epic's format).
  - `InitializeInput()` — as described in §4.3; iterates `GenericInputActions`, binds each to `HandleGenericInput` with tag payload. (Polish-language inline comment in header template: binding intent documented in code, not in doc comments.)
  - `HandleGenericInput()` — two sequential broadcasts (rich + simple delegates).
  - `GetInputConfigAsset()` — settings-driven soft-load with `LogTemp` warnings on failure (uses project placeholder names "UIInputConfig"/"UISettings" in the log text).

There are **no** other .cpp files, no editor module, no `log` categories defined (all logging is `LogTemp`), and no `BeginPlay`/`SetupInputComponent` overrides in the plugin — binding is opt-in via `InitializeInput()`.

## 6. Configuration (.ini)

**brak** — the plugin ships no .ini files (no `Config/*.ini` in the tree). Runtime configuration is:
- `UZWInputSettings` declared `UCLASS(Config=Game, defaultconfig)`, so its `InputConfig` property persists to the project's `DefaultGame.ini` under the `[DEFAULTCLASSPATH]`-style generated `/Script/ZWInput.ZWInputSettings` section (file name depends on user's project), and
- The selected `UZWInputConfig` soft asset (defined in **ZWCore**, not ZWInput) which itself holds the tag→action mapping data.

No console variables, config-only classes, or engine-scan-time config registration beyond `UDeveloperSettings`'s built-in behavior.

## 7. Dependencies within ZWSuite

| Dependency | Direction | Source |
|---|---|---|
| **ZWCore** | ZWInput depends on ZWCore (hard) | `.uplugin` plugin-list entry; Build.cs `PublicDependencyModuleNames` includes `"ZWCore"`; `ZWInputComponent.h` includes `"ZWInputConfig.h"` (class `UZWInputConfig`, `FZWInputAction`, `NativeInputActions`, `GenericInputActions` are **defined in ZWCore**, not in this plugin); `UZWInputSettings` refers to `UZWInputConfig` type. |
| **EnhancedInput** | External (engine plugin), required by `.uplugin` and Build.cs | Base class `UEnhancedInputComponent` and `ETriggerEvent`, `FInputActionValue`, `UInputAction` types. |
| **GameplayTags** | External (engine module) | `FGameplayTag`, `FGameplayTagContainer` in public API surface. |
| **UStateTree** | Engine `StateTreeModule` type is forward-declared in `ZWInputSettings.h` but **unused** in the header — vestigial / planned integration point. |
| **UZWInputConfig_Old** | Forward-declared legacy config type in `ZWInputSettings.h`, **unused** — indicates a deprecated vZwInputConfig type retained for reference/refactoring backwards. |

## 8. Notes / Risks

- **Hard dependency on ZWCore**: without ZWCore the Build.cs will fail to link (`ZWInputConfig.h` include path resolves into the ZWCore module's Public directory). Count: 1 required sibling plugin.
- **Synchronous asset loads**: `LoadSynchronous()` in `InitializeInput()`, `BindNativeAction()` and `GetInputConfigAsset()` — on the main thread; acceptable for small configs but can hitch on large config asset chains / streaming levels.
- **No auto-initialization**: `InitializeInput()` is not called by the plugin itself (no `BeginPlay`, no `SetupInputComponent` override, no PC attach helper). Users must remember to call it (typically from Pawn/Controller after `SetupPlayerInputComponent`). Silent no-op if config is unset — only a `LogTemp` warning surfaces misconfiguration.
- **Duplicated broadcast** to both `OnInputTagTriggered` and `OnInputTagSimpleTriggered` for every generic input — consumers subscribed to both will get two calls per input frame/event; make sure no listener subscribes to both.
- **`TagNot match case`**: `MatchesTagExact` is required for the native path (no parent-tag matching), while the generic path uses `ActionTag.IsValid()` (any tag accepted, no exact match filter) — semantics differ intentionally but can surprise users.
- **`UPROPERTY() const TSoftObjectPtr` indirection**: `CachedConfig` is a hard `UPROPERTY` pointer, so once loaded, the config asset stays resident for the component's lifetime — memory stays pinned even if the level unloads unless the component is destroyed.
- **Dead/vestigial code**: forward declarations `UStateTree` and `UZWInputConfig_Old` are unused; log messages reference outdated names ("UIInputConfig", "UISettings").
- **No .ini / no editor module / no tests / no localization**: entirely runtime-only; no input-buffering or `PlayerController` integration helpers yet.
- **No BlueprintScriptCallable UFUNCTIONs**: the only UCLASSes are exposed through reflection-typical `UPROPERTY` / editor settings — the component offers no Blueprint-callable function surface (bindings are C++-template only).
- Copyright headers are Epic Games template text; source includes Polish-language inline comments (e.g. `// Szukamy w naszym DataAssetcie akcji przypisanej do tego konkretnego Taga`) — authoring notes not for runtime.

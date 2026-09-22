# ZWCore — Technical Documentation

## 1. Overview

ZWCore is the foundational/base plugin of the ZWSuite (13 UE plugins, Felix P4 project). Its purpose is to provide shared, low-level assets and types used by the rest of the suite. Currently it contains exactly one gameplay-facing feature: the **Enhanced Input configuration Data Asset** (`UZWInputConfig`), which maps physical `UInputAction` assets (Enhanced Input) to logical `InputTag` GameplayTags plus a trigger event. This decouples gameplay/UI code from concrete `UInputAction` references: systems subscribe to tags like `UI.Action.Inventory` regardless of which input asset fires them.

Layer coverage: **Runtime game layer only**. The single module `ZWCore` is declared with `"Type": "Runtime"`, `LoadingPhase: Default` — there is no Editor module, no editor-only code, and no dedicated gameplay subsystem/component yet.

## 2. Metadata (`.uplugin`)

Source: `ZWCore.uplugin` (FileVersion 3).

| Key | Value |
|---|---|
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | `ZWCore` |
| Description | *(empty)* |
| Category | `Other` |
| CreatedBy / CreatedByURL / DocsURL / MarketplaceURL | *(empty)* |
| EnabledByDefault | `false` |
| CanContainContent | `true` |
| IsBetaVersion / IsExperimentalVersion | `false` |
| Installed | `false` |
| Icons | `Resources/Icon128.png` (only asset in Resources) |

Modules (1):

| Module | Type | LoadingPhase |
|---|---|---|
| `ZWCore` | Runtime | Default |

Plugin dependencies (`Plugins: []`):

| Name | Enabled |
|---|---|
| `EnhancedInput` | `true` |

## 3. Modules (Build.cs)

Source: `Source/ZWCore/ZWCore.Build.cs` — single module class `ZWCore : ModuleRules`, `PCHUsage = UseExplicitOrSharedPCHs`.

- **PublicDependencyModuleNames** (static link, public headers may expose these):
  - `Core`
  - `EnhancedInput`
  - `GameplayTags`
- **PrivateDependencyModuleNames** (static link, implementation only):
  - `CoreUObject`
  - `Engine`
  - `Slate`
  - `SlateCore`
- **PublicIncludePaths / PrivateIncludePaths / DynamicallyLoadedModuleNames**: none (empty — template placeholders only).

## 4. Public API

### 4.1 Module

Header: `Source/ZWCore/Public/ZWCore.h`

```
class FZWCoreModule : public IModuleInterface
{
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
};
```

Standard boilerplate module class; API macro `ZWCORE_API` is exported (used by `UZWInputConfig`).

### 4.2 Struct `FZWInputAction` (`USTRUCT`, `BlueprintType`)

Header: `Source/ZWCore/Public/ZWInputConfig.h`

Single mapping entry: Enhance Input action → GameplayTag. Template comment (Polish) reads: "Struktura pojedynczego mapowania (Akcja -> Tag)".

| UPROPERTY | Type | Specifiers | Purpose |
|---|---|---|---|
| `InputAction` | `TSoftObjectPtr<UInputAction>` | `EditDefaultsOnly, BlueprintReadOnly` | Physical Enhanced Input action (e.g. `IA_ToggleInventory`); soft reference to avoid hard asset loads |
| `InputTag` | `FGameplayTag` | `EditDefaultsOnly, BlueprintReadOnly, meta=(Categories="InputTag")` | Logical tag emitted on press (e.g. `Input.UI.Action.Inventory` per the meta filter `InputTag`); comment example: `"UI.Action.Inventory"` |
| `TriggerEvent` | `ETriggerEvent` | `EditDefaultsOnly, BlueprintReadOnly` | Defaults to `ETriggerEvent::Started`; which trigger state of the action emits the tag |

### 4.3 Class `UZWInputConfig` (`UCLASS`, base `UDataAsset`, `ZWCORE_API`, not Blueprintable/BlueprintType — specifiers are `UCLASS()` bare)

Config Data Asset holding two independent mapping lists. `TitleProperty = "InputAction"` on both arrays so list entries display by action name in Details panels.

| UPROPERTY | Type | Specifiers | Purpose |
|---|---|---|---|
| `NativeInputActions` | `TArray<FZWInputAction>` | `EditDefaultsOnly, BlueprintReadOnly, Category="Native Input", meta=(TitleProperty="InputAction")` | Character-possessed / native gameplay inputs (typical Lyra-style split) |
| `GenericInputActions` | `TArray<FZWInputAction>` | `EditDefaultsOnly, BlueprintReadOnly, Category="Generic Input", meta=(TitleProperty="InputAction")` | Paused/menu/UI-level inputs (e.g. toggle inventory while a pawn exists) |

Forward declarations present: `class UInputAction;`. Includes: `CoreMinimal.h`, `GameplayTagContainer.h`, `Engine/DataAsset.h`, `InputTriggers.h`.

No other public classes, interfaces, UENUMs, or free functions exist in the plugin ("brak").

## 5. Implementation (Private)

`Source/ZWCore/Private/ZWCore.cpp` (whole file):

- `FZWCoreModule::StartupModule()` / `ShutdownModule()` — both **empty bodies** (default Epic template). No logging, no registration.
- `IMPLEMENT_MODULE(FZWCoreModule, ZWCore)` — standard module registration.
- `LOCTEXT_NAMESPACE "FZWCoreModule"` is defined and immediately undef'd — unused.

There is **no .cpp corresponding to `ZWInputConfig.h`** — it is a pure header-only UCLASS/USTRUCT definition relying on the reflection system (instancing, editing, serialization); that is valid for a DataAsset. There is no built-in helper API (e.g. `GetInputTagFor(UInputAction*)` lookup, no async input binding subsystem) — consumers (other ZWSuite plugins) must interpret the arrays themselves.

Patterns used: **Data-only plugin** (Data Asset registry pattern); soft references (`TSoftObjectPtr`) for input actions; GameplayTags (`Categories="InputTag"` filter) for logic decoupling.

## 6. Configuration (`.ini`)

No `.ini` files ship with the plugin (no `Config/` directory exists). Section 6 content: **brak**.

## 7. Dependencies within ZWSuite

Outbound (what ZWCore uses):
- Engine plugins: `EnhancedInput` (declared in `.uplugin` and linked in Build.cs), `GameplayTags`, `Slate`/`SlateCore` (API-level but called from non-UI code — boilerplate carried over from the module template).
- Other ZWSuite plugins: **none** — no `#include` or Build.cs references any `ZW*` plugin. ZWCore is a pure leaf/dependency source.

Inbound (inferred consumers):
- Any suite plugin that implements the Enhanced Input binding loop (InputTag listening, actuating abilities/UI actions) will read `UZWInputConfig` assets; it is the canonical shared input-asset contract for ZWSuite. CanContainContent=true so designers can ship `.uasset` instances of `UZWInputConfig` inside the plugin's content folder, though the plugin repository currently has no Content directory reconstructed.

## 8. Notes / Risks

- **Minimal scope**: only `UZWInputConfig` / `FZWInputAction` exist; no subsystems, components, helper libs. If more "core" features are expected, they are not here.
- **No runtime logic** for the config mapping: every consumer must enumerate `NativeInputActions` / `GenericInputActions` manually; a central binding helper (subsystem) would be a natural future addition to avoid duplicated loops across ZWSuite plugins.
- **`GenericInputActions` vs `NativeInputActions`** semantics are only implied by categories, not enforced in code — consistency depends on convention.
- Empty `.uplugin` Description and Category "Other" — cosmetic metadata debt.
- Insufficiently-moderated soft refs: `TSoftObjectPtr` gives safety against hard loads, but nothing initializes/streams the input actions at binding time — the consumer must resolve them.
- `Slate`/`SlateCore` private deps and `GameplayTags` public dep with current code embracing pure reflection: `Slate*` deps are almost certainly leftover template boilerplate (no Slate usage exists) and can be pruned.
- Risk of drift: tag category filter `Categories="InputTag"` restricts editor picking to `InputTag.*` tags but does not enforce tag vocabulary across the 13-plugin suite.

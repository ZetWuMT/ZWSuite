# ZWUIInputBridge — Documentation

## 1. Overview

`ZWUIInputBridge` is a tiny, optional bridge plugin within the ZWSuite. Its single purpose is
to keep ZWUICore's `UZWInputConfig` reference (`UZWUISettings::InputConfig`) in sync with
ZWInput's (`UZWInputSettings::InputConfig`), so a project that uses **both** plugins has a
single source of truth for the active `UZWInputConfig` instead of two independently editable
copies.

Key design properties taken directly from the source:

- **One-way sync direction.** ZWInput is always the single source of truth; ZWUICore's copy is
  the mirror. The bridge never copies from UI → Input.
- **Fully decoupled design.** Neither `ZWInput` nor `ZWUICore` know this plugin exists — the
  bridge only reads/writes their existing, public `UPROPERTY` fields through their existing
  public API. It is never a hard dependency of either plugin; instead it *is* the plugin that
  declares hard dependencies on both.
- **In-memory only.** The sync writes to the in-memory CDO of `UZWUISettings` only; it never
  calls `SaveConfig()`/`UpdateDefaultConfigFile()`, so nothing is ever written back into
  ZWUICore's config `.ini`. If the bridge is later disabled, ZWUICore's ini is untouched and
  the plugin reverts to reading whatever was last saved there.
- **Opt-in.** `EnabledByDefault` is `false` and the `.uplugin` description explicitly says:
  enable only if the project uses BOTH ZWInput and ZWUICore; if only one (or neither) is used,
  leave the plugin disabled — both plugins have zero compile-time or runtime awareness of each
  other or of this bridge.

The plugin contains no content (`CanContainContent: false`), no assets, no gameplay logic —
just one runtime module with one class.

## 2. Metadata (`.uplugin`)

Source file: `ZWUIInputBridge.uplugin`

| Field | Value |
|---|---|
| `FileVersion` | 3 |
| `Version` | 1 |
| `VersionName` | `1.0` |
| `FriendlyName` | `ZWUIInputBridge` |
| `Category` | `ZW/Bridges` |
| `Description` | Optional bridge plugin keeping ZWUICore's `UZWInputConfig` reference in sync with ZWInput's, so both plugins use a single source of truth instead of two independent copies. Enable ONLY if the project uses BOTH ZWInput and ZWUICore; otherwise leave disabled. |
| `CreatedBy` | '' (empty) |
| `CreatedByURL` | '' (empty) |
| `DocsURL` | '' (empty) |
| `MarketplaceURL` | '' (empty) |
| `EnabledByDefault` | `false` |
| `CanContainContent` | `false` |
| `IsBetaVersion` | `false` |
| `IsExperimentalVersion` | `false` |
| `Installed` | `false` |

## 3. Modules (`Build.cs`)

Source file: `Source/ZWUIInputBridge/ZWUIInputBridge.Build.cs`

Single module: `ZWUIInputBridge`, class `ZWUIInputBridge : ModuleRules`.

Module type and loading phase (from `.uplugin` `Modules` array):

- **Name:** `ZWUIInputBridge`
- **Type:** `Runtime` → this is a **runtime-only** plugin (the module is compiled into
  packaged/cooked builds and its sync runs in the shipped game as well as the editor).
- **LoadingPhase:** `Default` — the module does not need to be present before engine init;
  see `StartupModule()` below for how it compensates by deferring its work to
  `FCoreDelegates::GetOnPostEngineInit()`.

Build rules:

| Property | Value |
|---|---|
| `PCHUsage` | `UseExplicitOrSharedPCHs` |
| `PublicDependencyModuleNames` | `Core` |
| `PrivateDependencyModuleNames` | `CoreUObject`, `Engine`, `DeveloperSettings`, `ZWInput`, `ZWUICore` |
| `DynamicallyLoadedModuleNames` | (empty) |

Notes:

- Everything of substance is a **private** dependency: the bridge never exposes ZWInput /
  ZWUICore types outwards, so including `ZWUIInputBridge` in another module does not leak
  either plugin's headers.
- `DeveloperSettings` is listed even though the currently wired code path only uses
  `FCoreDelegates` (the `UDeveloperSettings::OnSettingChanged()` subscription is commented
  out; see section 5). It exists for/by the disabled editor-only live-sync path.
- There are no build-version conditionals beyond the standard `WITH_EDITOR` usage in the
  initializer comments; the dependency lists are unconditioned.

## 4. Public API — Classes

Source file: `Source/ZWUIInputBridge/Public/ZWUIInputBridgeModule.h`

Total: **1 class**, exported by the module:

### `FZWUIInputBridgeModule : public IModuleInterface`

Lives in the public header of module `ZWUIInputBridge`. It is the module's sole class; the
header comment states its one job: mirror ZWInput's `InputConfig` into ZWUICore's, guaranteeing
a single source of truth whenever both plugins are enabled, and guaranteeing zero behavior
change when the plugin is disabled ("both settings objects behave exactly as if this module
never existed").

**Members:**

| Visibility | Member | Signature | Description |
|---|---|---|---|
| public | `StartupModule()` (override) | `virtual void StartupModule() override` | `IModuleInterface` startup hook. Registers the one-time sync on engine init and (disabled, editor-only) the live `UDeveloperSettings` change subscription. |
| public | `ShutdownModule()` (override) | `virtual void ShutdownModule() override` | `IModuleInterface` shutdown hook. Currently a no-op; contains only the commented-out unsubscription of `OnSettingChanged()`. |
| private (static) | `SyncInputConfig()` | `static void SyncInputConfig()` | Copies `UZWInputSettings::InputConfig` into `UZWUISettings::InputConfig` — **in memory only, never writes to config ini** (per header doc). |
| private (static, editor-only) | `OnAnySettingChanged()` | `static void OnAnySettingChanged(UObject* Settings)` | `#if WITH_EDITOR` handler reacting to any `UDeveloperSettings` CDO being edited in Project Settings; re-syncs only if the edited settings object is a `UZWInputSettings`. Member is **compiled but unused** in the shipped code (see section 5). |
| private (field, editor-only) | `SettingsChangedHandle` | `FDelegateHandle SettingsChangedHandle` | `#if WITH_EDITOR` stored delegate handle for the (currently commented-out) subscription; it is never assigned in the active code. |

No UCLASSes, no-reflected types, no delegates, no gameplay API — this is purely a UBT module
class. There is no `#include "MinimalAPI"`/export macro beyond the default module linkage for
an `IModuleInterface` implementation.

## 5. Implementation (Private)

Source file: `Source/ZWUIInputBridge/Private/ZWUIInputBridgeModule.cpp`

### Includes and setup

- Own header, then `ZWInputSettings.h` (from plugin **ZWInput**) and `ZWUISettings.h` (from
  plugin **ZWUICore**) — this is the concrete compile-time coupling point to both plugins.
- `Misc/CoreDelegates.h` for `GetOnPostEngineInit`.
- `Engine/DeveloperSettings.h` inside `#if WITH_EDITOR` only.
- `LOCTEXT_NAMESPACE "FZWUIInputBridgeModule"` defined/undef'd; no localized text is
  actually used.
- Ends with `IMPLEMENT_MODULE(FZWUIInputBridgeModule, ZWUIInputBridge)`.

### `StartupModule()`

```cpp
FCoreDelegates::GetOnPostEngineInit().AddStatic(&FZWUIInputBridgeModule::SyncInputConfig);
```

- Defers the sync until **engine init has fully completed**. Per the in-code comment this is
  safe in both editor and packaged games and guarantees both settings CDOs are fully loaded
  from their config files before the bridge touches them — necessary because the module loads
  at `LoadingPhase: Default`, before the settings objects are necessarily populated.
- Lives outside any `WITH_EDITOR` guard → the startup sync runs also in non-editor,
  runtime/packaged builds.

Then, `#if WITH_EDITOR`, the intended *live sync* path:

```cpp
//SettingsChangedHandle = UDeveloperSettings::OnSettingChanged().AddStatic(&FZWUIInputBridgeModule::OnAnySettingChanged);
```

- **Currently commented out.** The intent: when a designer edits `UZWInputSettings::InputConfig`
  in Project Settings while the editor is running, mirror the change into `UZWUISettings`
  immediately instead of waiting for an editor restart.
- The code carries an explicit portability/compatibility note: `UDeveloperSettings::OnSettingChanged()`
  is available on UE5; if the target engine version does not expose it, the whole `WITH_EDITOR`
  block can simply be deleted, since the `OnPostEngineInit` sync alone still covers editor and
  game startup (it just loses mid-session live updates).

### `ShutdownModule()`

Mirror of the disabled editor path:

```cpp
#if WITH_EDITOR
//UDeveloperSettings::OnSettingChanged().Remove(SettingsChangedHandle);
#endif
```

Effectively a no-op in the current code: there is nothing to unregister, because the static
`OnPostEngineInit` delegate is registered via `AddStatic` (a static multi-cast delegate) and
needs no explicit removal for module shutdown, and the `OnSettingChanged` subscription is
commented out.

### `SyncInputConfig()` — the core logic

```cpp
const UZWInputSettings* InputSettings = GetDefault<UZWInputSettings>();
UZWUISettings* UISettings = GetMutableDefault<UZWUISettings>();
if (InputSettings && UISettings)
{
    UISettings->InputConfig = InputSettings->InputConfig;
}
```

Behavior details:

- Reads the **class default objects (CDOs)** of both `UDeveloperSettings` subclasses,
  not per-world/instance objects — the sync is about project-wide default configuration.
- `UZWInputSettings` is read as `const`; `UZWUISettings` is fetched with
  `GetMutableDefault<>()` because it is the write target.
- **ZWInput is the documented single source of truth** for which `UZWInputConfig` is active
  whenever the bridge is enabled (in-code comment).
- The assignment changes **only the in-memory CDO value**. It does **not** call
  `SaveConfig()`/`UpdateDefaultConfigFile()`, so `DefaultZWUICore.ini` is never modified.
  If the bridge is later disabled, ZWUICore's config ini is untouched and the plugin reverts
  to reading whatever was last saved there.
- Hard null-guard: if either CDO is missing (e.g. a malformed suite install), it silently does
  nothing (no logging, no warnings).
- Declared `static`, so no `this` state is required at runtime.

### `OnAnySettingChanged()` (editor-only)

```cpp
if (Settings && Settings->IsA<UZWInputSettings>())
{
    SyncInputConfig();
}
```

- Intended callback for `UDeveloperSettings::OnSettingChanged()`: reacts to *any*
  `UDeveloperSettings` CDO being edited in Project Settings, filters with `IsA<UZWInputSettings>()`,
  and re-syncs. Note that every unrelated settings edit in Project Settings would therefore
  invoke this callback while it is subscribed.
- Presently dead code because the subscription is commented out — the member
  `SettingsChangedHandle` remains an unassigned `FDelegateHandle`.

### Runtime flow summary

1. Plugin `EnabledByDefault = false` → must be explicitly enabled in the project
   (`.uproject` / `Default*.ini` / plugins list) together with `ZWInput` and `ZWUICore`
   (which the `.uplugin` also marks as required plugins).
2. `LoadingPhase: Default` → module loads with other default-phase modules; no sync happens
   here.
3. Engine finishes init → `FCoreDelegates::OnPostEngineInit` fires → `SyncInputConfig()` runs
   once: `UZWUISettings::InputConfig := UZWInputSettings::InputConfig` (memory only).
4. All subsequent ZWUICore `UZWInputConfig` consumers thus read the same config as ZWInput.
5. No periodic, per-frame, or event-driven resync exists in the shipped code: a change to
   ZWInput's InputConfig after engine init is picked up on the next process start, not live
   (until/unless the editor-only `OnSettingChanged` wiring is re-enabled).

## 6. Configuration (`.ini`)

**(brak)** — the plugin ships no `.ini` files and registers no `UDeveloperSettings` class of
its own. There is no `Config/` directory anywhere under `Source/` (verified: the plugin tree
contains only `SWUIInputBridge.uplugin`, `Build.cs`, one public header, one private cpp, and
`Resources/Icon128.png`).

- No config reads/writes are performed by the module itself: `SyncInputConfig()` reads an
  already-config-loaded CDO property and writes another CDO property in memory only
  (explicitly without `SaveConfig()`/`UpdateDefaultConfigFile()`).
- Consequently the only `.ini` relevant to this plugin are those of its host plugins
  (`UZWInputSettings` for ZWInput, `UZWUISettings` for ZWUICore) and those are **not**
  modified by the bridge at any time.

## 7. Dependencies within ZWSuite

Declared plugin references in `.uplugin` `Plugins` array (both required):

| Plugin | `Enabled` | Relationship |
|---|---|---|
| `ZWInput` | `true` | Source of truth. The bridge reads `UZWInputSettings::InputConfig` (private `#include "ZWInputSettings.h"`). ZWInput itself has **zero** awareness of this bridge. |
| `ZWUICore` | `true` | Mirror / write target. The bridge writes `UZWUISettings::InputConfig` (private `#include "ZWUISettings.h"`). ZWUICore itself has **zero** awareness of this bridge. |

Build.cs coupling:

- `PrivateDependencyModuleNames` includes `ZWInput` and `ZWUICore` → compile-time dependency on
  both suite plugins exists **only inside this plugin**, and because they are private deps, it
  is not propagated outward to consumers of `ZWUIInputBridge`.
- `DeveloperSettings` (engine module) supports the settings-CDO infrastructure
  (`UDeveloperSettings` base types both settings classes share).

EnhancedInput relationship:

- The plugin source contains **no reference to EnhancedInput whatsoever** — no
  `EnhancedInput` module name in `Build.cs`, no fact that `UZWInputConfig` is Enhanced-Input-keyed
  appears or is used in the bridge code. The bridge is only concerned with *copying the
  object reference* (`TObjectPtr/UObject*` to `UZWInputConfig` CDO) from ZWInput settings to
  ZWUI settings; what that config object is keyed with (Enhanced Input or not) is entirely
  ZWInput/ZWUICore's concern and invisible to this bridge.

Dependency direction illustration:

```
ZWInput (owner)  ──read InputConfig──▶  ZWUIInputBridge (bridge)  ──write InputConfig──▶  ZWUICore (consumer)
      ▲                                                                                                │
      └────────────────────── no reverse dependency, no header mention, no task ───────────────────────┘
```

Nothing else in ZWSuite is documented by source within this plugin: there are no UI classes,
no GAS, no Unreal networking, no save/load or streaming code here.

## 8. Notes / Risks

1. **Single-shot sync only.** The shipped code syncs once at `OnPostEngineInit`; nothing
   re-syncs afterwards. If you change ZWInput's InputConfig at runtime (e.g. in a packaged
   game, or in the editor without restarting), ZWUICore will keep pointing at the
   last-synced config until the next engine start.
2. **Live mid-session sync is disabled.** The editor-only `UDeveloperSettings::OnSettingChanged()`
   wiring is fully commented out in both `StartupModule()` and `ShutdownModule()`, even though
   both the header (`OnAnySettingChanged`, `SettingsChangedHandle`) and the cpp have the
   (dead) code ready. Re-enabling requires uncommenting both sides plus the `#if WITH_EDITOR`
   includes, and requires the target engine to expose `UDeveloperSettings::OnSettingChanged()`
   (documented as available on UE5).
3. **No error / log surface.** If either CDO is missing, the sync silently no-ops. On a
   misconfigured install (e.g. one plugin disabled and the other enabled — which the
   `.uplugin`'s required-plugin list makes hard but not impossible), the failure mode is a
   silent mismatch, not an error.
4. **Runtime module cost.** The module is always active in shipped builds when enabled. Its
   runtime footprint is a single one-shot CDO property copy — effectively zero — but it does
   mean both suite plugins must link in packaged builds whenever this bridge is enabled.
5. **Editor convenience tradeoff.** Because the sync is not written to disk, a designer
   looking at `DefaultZWUICore.ini` mid-session sees the *stale* UI-side value even while the
   in-memory copy is freshly mirrored. This is a deliberate tradeoff ("disk is untouched",
   two-way divergence impossible from UI → Input) but can be visually confusing to inspect.
6. **No content, no tests, no `.ini`, no third-party deps.** `CanContainContent: false`, no
   automated tests exist, no configuration surface, no external dependencies beyond the
   engine-level `Core/CoreUObject/Engine/DeveloperSettings`.
7. **Single class / single file module.** Reading the whole plugin takes ~170 lines; any
   substantive future change (e.g. bi-directional sync or editor-only compile enforcement)
   will fit in one file.

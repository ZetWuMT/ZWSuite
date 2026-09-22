# ZWGASExtensions — Dokumentacja techniczna (Technical Documentation)

## 1. Przegląd (Overview)

**ZWGASExtensions** is a small Unreal Engine plugin in the ZWSuite family that extends Epic's **Gameplay Ability System (GAS)** — specifically the `AbilitySystemComponent` — to wire GAS ability activation directly into the **ZWInput** input-tag system.

The plugin provides exactly one gameplay class, `UZWAbilitySystemComponent` (a subclass of `UAbilitySystemComponent`). When a `PlayerController` is assigned to the component (`OnPlayerControllerSet`), it looks up the controller's `UZWInputComponent` and subscribes to the `OnInputTagSimpleTriggered` delegate. Each triggered `FGameplayTag` is forwarded to `HandleInputTag`, which calls `TryActivateAbilitiesByTag` on a one-tag container — i.e. **an input tag press directly activates every granted ability matching that tag**. On component destruction it unsubscribes from the input component's `OnInputTagTriggered` delegate to avoid dangling callbacks.

In short: it is the bridge between ZWInput (tag-based input routing) and GAS ability activation, so that abilities can be authored entirely by gameplay tags without per-ability input bindings.

Key facts:

- Single runtime module, also named `ZWGASExtensions`.
- Depends on the `GameplayAbilities` plugin and the sibling ZWSuite plugin `ZWInput`.
- `CanContainContent: true` (may ship assets), though no Content directory currently exists in the source tree (only `Resources/Icon128.png`).
- Module-level `StartupModule` / `ShutdownModule` are empty stubs (default plugin template code); all real logic lives in the component class.

## 2. Metadane (.uplugin)

Source: `ZWGASExtensions.uplugin`

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | 1.0 |
| FriendlyName | `ZWGASExtensions` |
| Description | *(empty string — brak/not set)* |
| Category | `Other` |
| CreatedBy | `tiramisoo` |
| CreatedByURL | *(empty — brak)* |
| DocsURL | *(empty — brak)* |
| MarketplaceURL | *(empty — brak)* |
| CanContainContent | `true` |
| IsBetaVersion | `false` |
| IsExperimentalVersion | `false` |
| Installed | `false` |

**Modules** (1 total):

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWGASExtensions` | Runtime | Default |

**Plugin dependencies** (declared in `.uplugin` → `Plugins`):

| Name | Enabled |
|---|---|
| `GameplayAbilities` | true |
| `ZWInput` | true |

Resources: `Resources/Icon128.png` (plugin icon). No other assets ship in this plugin.

## 3. Podmoduły (Build.cs)

Single module: `Source/ZWGASExtensions/ZWGASExtensions.Build.cs`, class `ZWGASExtensions : ModuleRules`.

- **PCHUsage**: `UseExplicitOrSharedPCHs`.
- **PublicIncludePaths**: none (empty template list).
- **PrivateIncludePaths**: none (empty template list).

**PublicDependencyModuleNames** (public/static link):

- `Core`
- `GameplayAbilities` — the GAS module; `AbilitySystemComponent.h`, `FGameplayTag`, ability activation APIs
- `GameplayTags`
- `GameplayTasks`

**PrivateDependencyModuleNames** (implementation-only):

- `CoreUObject`
- `Engine`
- `Slate`
- `SlateCore`
- `ZWInput` — sibling ZWSuite plugin; supplies `UZWInputComponent` and its input-tag delegates

**DynamicallyLoadedModuleNames**: none.

Notes / observations:

- `Engine`, `Slate` and `SlateCore` are private dependencies that come from the default plugin template; `Engine` is arguably needed publicly because the class header is in `Public/`, but as written the header compiles against public GAS deps only, so this is consistent with the include set (`CoreMinimal.h`, `AbilitySystemComponent.h`).
- `ZWInput` is only a private dependency because `UZWInputComponent` is referenced exclusively from the `.cpp` (the public header forward-declares `UZWAbilitiesConfig` instead, which is unused in the current code — see §8).

## 4. Publiczny API — klasy

The plugin's public API surface lives under `Source/ZWGASExtensions/Public/` in two headers.

### 4.1 `UZWAbilitySystemComponent` (`Public/ZWAbilitySystemComponent.h`)

- **Base class**: `UAbilitySystemComponent` (engine, GameplayAbilities plugin)
- **API macro**: `ZWGASEXTENSIONS_API`
- **UCLASS specifiers**: `UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))` — the component is spawnable and usable in Blueprints.
- **Forward declarations**: `class UZWAbilitiesConfig;` (declared but never used in the current implementation — residual/wip reference; see §8).

#### Members

| Kind | Signature | Notes |
|---|---|---|
| virtual override | `void OnComponentDestroyed(bool bDestroyingHierarchy)` | Calls `Super::` then unbinds this component from the owning player controller's `UZWInputComponent::OnInputTagTriggered` via `RemoveAll(this)`. Guards with `AbilityActorInfo->PlayerController` validity and a `Cast<UZWInputComponent>` on `PlayerController->InputComponent`. |
| virtual override | `void OnPlayerControllerSet()` | Calls `Super::` then locates `UZWInputComponent` on the newly set player controller via `GetComponentByClass<UZWInputComponent>()` and binds `OnInputTagSimpleTriggered` → `HandleInputTag` (`AddUObject`). Not bulletproof-guarded: only the weak-pointer (`PlayerController.IsValid()`) is checked, no cast needed since the type is looked up. |
| BlueprintCallable UFUNCTION | `void HandleInputTag(FGameplayTag InGameplayTag)` | Category **"ZWInput"**. Wraps the incoming tag into `FGameplayTagContainer(InGameplayTag)` and calls `TryActivateAbilitiesByTag(...)` — activating all granted abilities whose activation-owned tags match. This is the bridge from input tags to GAS abilities. |

UPROPERTY members: **none** (the class exposes no replicated data, no config, no editor-visible properties).

#### Runtime wiring summary

1. GAS gives the component an `AbilityActorInfo` with a `PlayerController` → engine calls `OnPlayerControllerSet()`.
2. The component finds the controller's `UZWInputComponent` (from the `ZWInput` plugin) and subscribes to `OnInputTagSimpleTriggered`.
3. Each triggered `FGameplayTag` flows into `HandleInputTag` → `TryActivateAbilitiesByTag`.
4. On destruction, the component unsubscribes (note: it unsubscribes from `OnInputTagTriggered`, but the *bound* delegate is `OnInputTagSimpleTriggered` — see §8 for this asymmetry).

### 4.2 `FZWGASExtensionsModule` (`Public/ZWGASExtensions.h`)

- **Base class**: `IModuleInterface` (engine `Modules/ModuleManager.h`)
- **Purpose**: standard plugin module lifecycle shell; no project-specific logic.
- **Members** (plain virtuals, no UHT reflection):
  - `virtual void StartupModule() override;` — empty body (template stub).
  - `virtual void ShutdownModule() override;` — empty body (template stub).
- Implemented in `Private/ZWGASExtensions.cpp` with `LOCTEXT_NAMESPACE "FZWGASExtensionsModule"` and registered via `IMPLEMENT_MODULE(FZWGASExtensionsModule, ZWGASExtensions)`.

There are no other public headers, no enums, no structs, no interfaces, no delegates declared in this plugin.

## 5. Implementacja (Private)

Private sources live under `Source/ZWGASExtensions/Private/`:

### `Private/ZWAbilitySystemComponent.cpp`

```cpp
void UZWAbilitySystemComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
    Super::OnComponentDestroyed(bDestroyingHierarchy);
    TWeakObjectPtr<APlayerController> PlayerController = AbilityActorInfo->PlayerController;
    if (PlayerController.IsValid())
    {
        if (UZWInputComponent* InputComponent = Cast<UZWInputComponent>(PlayerController->InputComponent))
        {
            InputComponent->OnInputTagTriggered.RemoveAll(this);
        }
    }
}

void UZWAbilitySystemComponent::OnPlayerControllerSet()
{
    Super::OnPlayerControllerSet();
    TWeakObjectPtr<APlayerController> PlayerController = AbilityActorInfo->PlayerController;
    if (PlayerController.IsValid())
    {
        UZWInputComponent* InputComponent = PlayerController.Get()->GetComponentByClass<UZWInputComponent>();
        if (InputComponent)
        {
            InputComponent->OnInputTagSimpleTriggered.AddUObject(this, &UZWAbilitySystemComponent::HandleInputTag);
        }
    }
}

void UZWAbilitySystemComponent::HandleInputTag(FGameplayTag InGameplayTag)
{
    TryActivateAbilitiesByTag(FGameplayTagContainer(InGameplayTag));
}
```

Implementation details worth noting:

- Both controller-access paths read from `AbilityActorInfo->PlayerController` (a `TWeakObjectPtr<APlayerController>`), relying on the ability-system info being populated by the parent `UAbilitySystemComponent` initialization flow.
- Input-component lookup differs between the two hooks: `OnPlayerControllerSet` uses `GetComponentByClass<UZWInputComponent>()` (i.e. the ZW input component is expected to be a **UActorComponent** on the controller), whereas `OnComponentDestroyed` does a `Cast<UZWInputComponent>` on `PlayerController->InputComponent` (the legacy TLazyObjectPtr input slot). These two assumptions are not guaranteed to return the same object — see §8.
- `RemoveAll(this)` is used for unbinding, which cleanly removes every binding owned by this component regardless of delegate handle tracking.
- No logging, no validation, no ifdef'd editor-only code, no networking/replication code in this file.

### `Private/ZWGASExtensions.cpp`

Standard module boilerplate: declares `LOCTEXT_NAMESPACE "FZWGASExtensionsModule"` (undefined at the end) and empty `StartupModule` / `ShutdownModule` implementations. Registered with `IMPLEMENT_MODULE(FZWGASExtensionsModule, ZWGASExtensions)`. No init-time work is performed — the plugin has no global state, no delegates, no settings objects registered at startup.

## 6. Konfiguracja (.ini)

**brak** — the plugin contains no `Config/` directory, no `.ini` files, no `UDeveloperSettings` class, and no config-property (`config=`, `Config`/`GlobalUserConfig` UPROPERTY) usage anywhere in the code. All behavior is hardcoded in `UZWAbilitySystemComponent`; nothing is user-configurable short of subclassing in C++.

## 7. Zależności wewnątrz ZWSuite

- **`ZWInput` (hard dependency, both `.uplugin` and Build.cs)**: `UZWAbilitySystemComponent` directly consumes:
  - `UZWInputComponent` (class from ZWInput) — resolved either via `GetComponentByClass` on the PlayerController or via casting the controller's `InputComponent`.
  - `UZWInputComponent::OnInputTagSimpleTriggered` — delegate this component subscribes to on controller assignment (the main inbound path for input tags).
  - `UZWInputComponent::OnInputTagTriggered` — delegate this component unsubscribes from on destruction (cleanup path).
  - The BlueprintCallable category is `"ZWInput"`, aligning the API naming with that plugin.
- **Other ZWSuite plugins**: none referenced. No other `ZW*` headers or symbols appear anywhere in this plugin.
- The forward-declared `UZWAbilitiesConfig` in the public header suggests an intended link to a config class (possibly from another ZWSuite plugin or planned here), but it is **unused and unresolved by any current file in this plugin** — nothing in the build script suggests a hidden dependency either.

## 8. Uwagi / ryzyka

- **Delegate mismatch on cleanup**: `OnPlayerControllerSet()` binds to `OnInputTagSimpleTriggered`, but `OnComponentDestroyed()` clears `OnInputTagTriggered`. If these are distinct delegates in `UZWInputComponent`, the destructor does **not** actually remove the `HandleInputTag` binding → potential dangling delegate callback / use-after-destroy risk when the ASC is destroyed while the PlayerController (and its input component) outlives it. Highly likely a bug; verify against `ZWInput`'s `UZWInputComponent` header and unify the delegate names (or call `RemoveAll` on the same delegate that was bound).
- **Inconsistent input-component lookup**: the bind path uses `GetComponentByClass<UZWInputComponent>()` (a component on the controller), the cleanup path uses `Cast<UZWInputComponent>(PlayerController->InputComponent)` (the legacy inline input slot). If `UZWInputComponent` lives as a component but is not also assigned to `PlayerController->InputComponent`, the cleanup `RemoveAll` becomes a no-op — further weakening the destructor path.
- **Unhandled cast failure**: if the controller's input component is not (or does not also contain) a `UZWInputComponent`, the plugin silently does nothing — no warning/mock fallback is provided. Debugging benefits from a check/assert here.
- **`UZWAbilitiesConfig` dead forward declaration**: the public header forward-declares `class UZWAbilitiesConfig;` but never uses it. This will not compile-time break anything, but it implies either leftover scaffolding or an interrupted feature (likely an ability-config asset the plugin was intended to consume). Flag it as either dead code or work-in-progress; currently it is superfluous.
- **Description/DocsURL/MarketplaceURL empty in `.uplugin`**: distributable metadata is incomplete — fine for internal use, should be filled before any marketplace or team-wide release.
- **No networking consideration**: `OnPlayerControllerSet` and the input binding path assume a locally authoritative player. There is no replication guard (`IsLocallyControlled`, `ROLE_Authority` checks, etc.). On a dedicated server the input-delegate path simply never fires, which is safe, but there is no explicit protection if this component is placed on a remote/player代理 actor incorrectly.
- **`AbilityActorInfo` null-safety**: both hooks dereference `AbilityActorInfo->PlayerController` without checking `AbilityActorInfo` itself for null. In practice the parent class populates it before these callbacks fire, but a defensive null check would still be cheap insurance.
- **`TryActivateAbilitiesByTag` semantics**: wraps the single tag in a one-element container and relies on the default (non-exact-match) matching behavior; if an ability has a broader activation-owned tag set, unexpected activations may occur. Consider whether exact-match (`FGameplayTagContainers` exact param) is intended.
- **No tests, no editor-only module, no Content assets**: the plugin ships `Icon128.png` only. There is no CI/test coverage present in the source tree.

## 9. Podsumowanie rozmiaru / widełki

- Plugin files: `.uplugin` (1), `Build.cs` (1), public headers (2), private sources (2), icon (1) = 7 files.
- Modules: 1 runtime module (`ZWGASExtensions`, LoadingPhase Default).
- Reflected gameplay classes: 1 (`UZWAbilitySystemComponent`).
- Module-lifecycle classes: 1 (`FZWGASExtensionsModule`).
- Total documented classes: 2.
- UPROPERTY: 0. UFUNCTION: 1 (`HandleInputTag`). Virtual overrides: 2 (`OnComponentDestroyed`, `OnPlayerControllerSet`).

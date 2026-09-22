# ZWInteraction — Dokumentacja techniczna

## 1. Przegląd

**ZWInteraction** is a runtime Unreal Engine plugin (module name `ZWInteraction`, plugin version 1.0, category `Other`, authored by `tiramisoo`) that implements a player-facing **interaction system**: detecting, highlighting, inspecting and investigating interactable world objects.

The plugin is built around five cooperating pieces:

| Piece | Class | Role |
|---|---|---|
| Interactable marker | `UZWInteractionComponent` (ActorComponent) | Lives on any interactable actor; exposes `Interact()` / `OnInteract`, provides custom-depth highlight, and configures the owner's mesh collision response for the interaction trace channel. |
| Interaction detector | `UZWInteractionPlayerComponent` (UArrowComponent) | Lives on the player; ticks every frame, runs a sphere trace from the viewport center along the configured collision channel, and highlights the currently hit interactable. Calls `Interact()` on it. |
| Session state machine | `UZWInteractionSubsystem` (UGameInstanceSubsystem) | Global per-game-instance state for *Inspection* and *Investigation* modes; manages input-mode delegation, a cloned inspect actor, camera view-target switching, and multicast delegates for gameplay/UI integration. |
| Inspection render stage | `AZWInteractionSceneCapture` (ASceneCapture2D) | A self-contained off-scene studio (capture component + static mesh dock + cube backdrop + rect light + post process) that renders the cloned inspected actor to `RT_InspectionViewRenderTarget`. |
| Config | `UZWInteractionSystemSettings`, `UZWInteractionSystem_Settings` | Editor-facing DeveloperSettings (collision channel) and a plain `UObject` config class (investigation input mapping context). |

An interaction **interface** (`UZWInteractionInterface` / `IZWInteractionInterface`) also exists but is explicitly marked **"BYPASSED FOR NOW"** — the component-based path is the active one.

Design highlights:
- **Detection** is a player-side per-tick single sphere trace (`UKismetSystemLibrary::SphereTraceSingle`) on a project-configurable trace channel.
- **Highlighting** uses the custom-depth/stencil post-process pipeline (`SetRenderCustomDepth` + `SetCustomDepthStencilValue(1)`), overridable via `ToggleHighlight` (BlueprintOverridable).
- **Inspection** spawns (clones) the actor via template spawn, detaches it visually into the scene capture's "show only" set, and renders it at a fixed off-stage location `(0,0,-2000)`.
- **Investigation** is a stack of interacted objects ("camera targets") with view-target blends and mouse-look rotation offsets, suitable for e.g. examining a wall/room from a fixed camera.

## 2. Metadane (.uplugin)

File: `ZWInteraction.uplugin`

| Field | Value |
|---|---|
| `FileVersion` | 3 |
| `Version` | 1 |
| `VersionName` | 1.0 |
| `FriendlyName` | ZWInteraction |
| `Description` | (empty string — none provided) |
| `Category` | Other |
| `CreatedBy` | tiramisoo |
| `CreatedByURL` | (empty) |
| `DocsURL` | (empty) |
| `MarketplaceURL` | (empty)|
| `EnabledByDefault` | false |
| `CanContainContent` | **true** (plugin ships content, e.g. the required `RT_InspectionViewRenderTarget` asset) |
| `IsBetaVersion` | false |
| `IsExperimentalVersion` | false |
| `Installed` | false |

**Modules** (1):
| Name | Type | LoadingPhase |
|---|---|---|
| `ZWInteraction` | Runtime | Default |

**Plugin dependencies** (declared in .uplugin):
- `CommonUI` (Enabled)
- `EnhancedInput` (Enabled)

## 3. Podmoduły (Build.cs)

File: `Source/ZWInteraction/ZWInteraction.Build.cs` — one rules class `ZWInteraction : ModuleRules` (the plugin has a single runtime module).

| Setting | Value |
|---|---|
| `PCHUsage` | `UseExplicitOrSharedPCHs` |
| `PublicIncludePaths` | (none) |
| `PrivateIncludePaths` | (none) |
| `DynamicallyLoadedModuleNames` | (none) |

**Public dependencies** (statically linked, headers may be used in API):
- `Core`
- `EnhancedInput`
- `DeveloperSettings`

**Private dependencies:**
- `CoreUObject`
- `Engine`
- `Slate`
- `SlateCore`
- `CommonInput`
- `CommonUI`
- `GameplayTags`

Note: `GameplayTags` and `CommonInput`/`CommonUI` are listed as *private* although `UZWInteractionSystem_Settings.h` includes `InputMappingContext.h` (EnhancedInput module, public) — fine. The `CommonUI`/`CommonInput` linkage backs the subsystem (`CommonInputModeTypes.h`, `CommonInputSubsystem.h`, `CommonUIActionRouterBase.h` includes) and the EnhancedInput mapping-context handling; however, as of this source snapshot those subsystem includes are dead — see §8.

## 4. Publiczny API — klasy

The plugin defines **8 public C++ classes** (7 UCLASSes/UInterface pairs counted individually below, plus the module class). All are exported with `ZWINTERACTION_API`.

### 4.1 `FZWInteractionModule` — `ZWInteraction.h`
- **Base:** `IModuleInterface`. Module registration via `IMPLEMENT_MODULE(FZWInteractionModule, ZWInteraction)`.
- `virtual void StartupModule() override;` — empty (no-op).
- `virtual void ShutdownModule() override;` — empty (no-op).

### 4.2 `UZWInteractionInterface` / `IZWInteractionInterface` — `ZWInteractionInterface.h/.cpp`
- `UZWInteractionInterface` — `UInterface`, `MinimalAPI`, no `GENERATED_BODY` members ("does not need to be modified").
- **`IZWInteractionInterface` — base: none (plain interface class). Header comment: `/** BYPASSED FOR NOW */`.** All methods have empty default implementations in the .cpp (`Interact`, `Inspect`, `Investigate`, `ToggleHighlight` no-op; `IsInspectable`/`IsInvestigatable`/`IsInvestigationExclusive` return `false`).

| Member | Signature | Purpose |
|---|---|---|
| `Interact` | `virtual void Interact()` | Perform the primary interaction (default no-op). |
| `Inspect` | `virtual void Inspect()` | Enter inspection of the implementer (default no-op). |
| `Investigate` | `virtual void Investigate()` | Enter investigation of the implementer (default no-op). |
| `IsInspectable` | `virtual bool IsInspectable()` | Capability query (default `false`). |
| `IsInvestigatable` | `virtual bool IsInvestigatable()` | Capability query (default `false`). |
| `IsInvestigationExclusive` | `virtual bool IsInvestigationExclusive()` | Whether investigation excludes other interactions (default `false`). |
| `ToggleHighlight` | `virtual void ToggleHighlight(bool isHighlighted)` | Visual cue switch (default no-op). |
| `BPToggleHighlight` | `UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Interaction System") void BPToggleHighlight(bool isHighlighted)` | Blueprint-callable/native-event override hook for highlighting. **Note:** declared inside `IZWInteractionInterface` with a `UFUNCTION()` but the enclosing interface class is not a `UINTERFACE`-reflected class body with UHT support for non-UFUNCTION members — this compiles only because the pure-virtual members are non-UFUNCTION; `BPToggleHighlight` has no generated body/thunk re ⚠ see §8. |

### 4.3 `UZWInteractionComponent` — `ZWInteractionComponent.h/.cpp`
- **Base:** `UActorComponent`. `UCLASS(meta = (BlueprintSpawnableComponent))`.
- **Constructor:** `PrimaryComponentTick.bCanEverTick = false;` (event-driven; heavy lifting happens on the player component's tick).

| Member | Signature / Type | Kind | Purpose |
|---|---|---|---|
| ctor | `UZWInteractionComponent()` | — | Disable ticking. |
| `BeginPlay` | `virtual void BeginPlay() override` | lifecycle | Caches the owner's `UStaticMeshComponent` / `USkeletalMeshComponent`, sets stencil value 1 and the configured collision response. |
| `Interact` | `UFUNCTION(BlueprintCallable) void Interact()` | function | `OnInteract.Broadcast()` — the sole notification entry point. |
| `ToggleHighlight` | `UFUNCTION(Blueprintable) virtual void ToggleHighlight(bool bIsHighlighted)` | virtual, BlueprintOverridable | Default implementation: `SetRenderCustomDepth(IsHighlighted)` on the cached static and skeletal mesh components. Docs comment: "This function determines how the game is highlighting the interactable object… Override this function to set up the way you are providing the visual cue…" |
| `IsHighlighted` | `bool IsHighlighted() const { return bIsHighlighted; }` | inline | Current highlight state. |
| `OnInteract` | `FOnInteractDelegate` (`DECLARE_DYNAMIC_MULTICAST_DELEGATE`, `UPROPERTY(BlueprintAssignable, Category="InteractionSystem")`) | delegate | "The event called when the object is interacted with in any way." |
| `bIsHighlighted` | `bool bIsHighlighted = false` (private, non-UPROPERTY) | state | Cached highlight flag. |
| `StaticMeshComponent` | `UPROPERTY(Transient) UStaticMeshComponent*` (private) | cached | Owner's static mesh used for highlight + collision setup. |
| `SkeletalMeshComponent` | `UPROPERTY(Transient) USkeletalMeshComponent*` (private) | cached | Owner's skeletal mesh used for highlight + collision setup. |

**`BeginPlay` details (exact behavior):**
1. For `UStaticMeshComponent`: first `Owner->GetComponentByClass(UStaticMeshComponent::StaticClass())`; if `nullptr`, falls back to `Owner->FindComponentByClass(...)` filtered by tag `MainInteractionMesh` (`FindComponentByTag(UStaticMeshComponent::StaticClass(), FName(TEXT("MainInteractionMesh")))`).
2. Casts to `UStaticMeshComponent`; if found: `SetCustomDepthStencilValue(1)` and, if the project default `UZWInteractionSystemSettings` is available, `SetCollisionResponseToChannel(Settings->InteractionCollisionChannel, ECR_Block)` — this is what makes the owner visible to the detection trace.
3. Identical sequence for `USkeletalMeshComponent`.

### 4.4 `UZWInteractionPlayerComponent` — `ZWInteractionPlayerComponent.h/.cpp`
- **Base:** `UArrowComponent` (a PrimitiveComponent used as the "detector" origin/direction gizmo). `UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))`.
- Ticks every frame (`PrimaryComponentTick.bCanEverTick = true` in ctor).

| Member | Signature / Type | Kind | Purpose |
|---|---|---|---|
| ctor | `UZWInteractionPlayerComponent()` | — | Enables tick. |
| `TickComponent` | `virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override` | lifecycle | Calls `DetectInteractiveObjects()` every tick. |
| `Interact` | `UFUNCTION(BlueprintCallable) virtual void Interact()` | function | If `InteractableObject` is set **and highlighted**, forwards `InteractionComponent->Interact()`. |
| `ResolveLineTracePoints` | `virtual void ResolveLineTracePoints(FVector& TraceStart, FVector& TraceEnd, float& TraceRadius)` (protected, virtual) | hook | Default: falls back to `GetComponentLocation()/GetForwardVector(GetComponentRotation())`, then — if the owner's `GetInstigatorController()` casts to `APlayerController` — overrides with a deprojection of the **viewport center** (`PC->GetViewportSize`, `PC->DeprojectScreenPositionToWorld(X*0.5f, Y*0.5f, ...)`). `TraceEnd = TraceStart + Direction * DetectionRange`, `TraceRadius = DetectionTraceRadius`. |
| `ResolveLineTraceIgnoredActors` | `virtual void ResolveLineTraceIgnoredActors(TArray<AActor*>& ActorsToIgnore)` (protected, virtual) | hook | Default no-op ("Do nothing"); `DetectInteractiveObjects` separately adds `GetOwner()` before calling this. |
| `GetInteractableObjectInteractionComponent` | `virtual UZWInteractionComponent* GetInteractableObjectInteractionComponent(AActor* InteractableActor)` (protected, virtual) | hook | `InteractableActor->GetComponentByClass<UZWInteractionComponent>()` after a validity check. |
| `GetInteractableObject` | `TObjectPtr<UZWInteractionComponent> GetInteractableObject()` (inline) | getter | Currently highlighted interactable under trace. |
| `SetInteractableObject` | `void SetInteractableObject(TObjectPtr<UZWInteractionComponent> Object)` (protected) | setter | Assigns only if `IsValid(Object)`. |
| `ResetInteractableObject` | `void ResetInteractableObject()` (protected) | setter | If set: `ToggleHighlight(false)`, then clear. |
| `GetInteractedObject` | `TObjectPtr<UZWInteractionComponent> GetInteractedObject()` (inline) | getter | Last interacted object (no clear-on-streamout; reset per interact/inspect flow). |
| `SetInteractedObject` | `void SetInteractedObject(TObjectPtr<UZWInteractionComponent> Object)` (protected) | setter | Unconditional assignment. |
| `ResetInteractedObject` | `void ResetInteractedObject()` (protected) | setter | Clears without unhighlight. |
| `DetectInteractiveObjects` (private) | `void DetectInteractiveObjects()` | private | Runs the sphere trace and resolves/ resets the interactable. |
| `ResolveInteractiveObject` (private) | `void ResolveInteractiveObject(AActor* NewInteractableObject)` | private | See below. |

**Editable properties** (all `EditDefaultsOnly`, `Category="Interaction"`):
| Name | Type | Default | Meta | Purpose |
|---|---|---|---|---|
| `DetectionTraceRadius` | `float` | `7.f` | `ClampMin=0, UIMin=0, ClampMax=20, UIMax=20` | Sphere trace radius. |
| `DetectionRange` | `float` | `400.f` | `ClampMin=0, UIMin=0, ClampMax=2000, UIMax=2000` | Trace distance. |
| `bDrawDebugTrace` | `bool` | `false` | — | When true draws the trace `ForDuration`. |

**Private UPROPERTY state:** `InteractableObject` and `InteractedObject`, both `UPROPERTY() TObjectPtr<UZWInteractionComponent>` (null default).

**`DetectInteractiveObjects` details (exact behavior):**
1. `ActorsToIgnore` = `[GetOwner()]`; then `ResolveLineTracePoints` and `ResolveLineTraceIgnoredActors`.
2. Reads `GetDefault<UZWInteractionSystemSettings>()`; **early-returns silently if settings are null**, otherwise converts `Settings->InteractionCollisionChannel` via `UEngineTypes::ConvertToTraceType`.
3. `UKismetSystemLibrary::SphereTraceSingle(World, TraceStart, TraceEnd, DetectionTraceRadius, CollisionTrace, /*bTraceComplex*/false, ActorsToIgnore, debug, HitResult, /*bIgnoreSelf*/true)`.
   - Note the radius argument is `DetectionTraceRadius` again, though `ResolveLineTracePoints` also writes `TraceRadius = DetectionTraceRadius` into the out param — the member is effectively the single source of truth.
4. On hit → `ResolveInteractiveObject(HitActor)`; on miss and if an `InteractableObject` exists → `ResetInteractableObject()`.

**`ResolveInteractiveObject` details:**
1. `ResetInteractableObject()` (un-highlights previous).
2. If the currently `InteractedObject`'s owner equals the new hit actor → return (do not re-highlight the object currently being interacted with).
3. Otherwise fetch the `UZWInteractionComponent`; if found and `IsActive()`, `SetInteractableObject` + `ToggleHighlight(true)`; else `ResetInteractableObject()` again (explicit guard against components that are not active).

### 4.5 `UZWInteractionSubsystem` — `ZWInteractionSubsystem.h/.cpp`
- **Base:** `UGameInstanceSubsystem` (lifetime = per game instance; safe place for cross-mode state).

**Enums & delegates (public, in header):**
| Name | Kind | Definition |
|---|---|---|
| `EInteractionInputMode` | `UENUM(), uint8` | `NoInteraction`, `Inspection`, `Interface`, `Investigation` |
| `FSetInputModeDelegate` | `DECLARE_DELEGATE_OneParam` | `void(EInteractionInputMode)` — **bound by external code** (UI/system wires game modes in). |
| `FStartInspectionDelegate` | `DECLARE_MULTICAST_DELEGATE_OneParam` | `void(bool)` — parameter: whether the inspected item rotates. |
| `FEndInspectionDelegate` | `DECLARE_MULTICAST_DELEGATE` | `void()` |
| `FOnInvestigationStarted` | `DECLARE_MULTICAST_DELEGATE_OneParam` | `void(AActor*)` |
| `FOnInvestigationEnded` | `DECLARE_MULTICAST_DELEGATE` | `void()` |
| `FOnInvestigationViewTargetChanged` | `DECLARE_MULTICAST_DELEGATE_OneParam` | `void(AActor*)` |

**Public members:**

| Member | Signature / Type | Purpose |
|---|---|---|
| `SetInputMode` | `FSetInputModeDelegate` (public field) | Executed with `Inspection` / `Investigation` / `NoInteraction` as the system switches modes. Must be externally bound. |
| `StartInspectionDelegate` | `FStartInspectionDelegate` (public field) | Fires when inspection begins, param `IsRotatable`. |
| `EndInspectionDelegate` | `FEndInspectionDelegate` | Fires when inspection ends. |
| `OnInvestigationStarted` | `FOnInvestigationStarted` | Fires with the initial investigation target. |
| `OnInvestigationEnded` | `FOnInvestigationEnded` | Fires at end of investigation, after camera return. |
| `OnInvestigationViewTargetChanged` | `FOnInvestigationViewTargetChanged` | Fires whenever `SetCamera` completes. |
| `IsPlayerInteracting` | `UFUNCTION(BlueprintCallable) bool` | `bIsPlayerInteracting` (no setter in this file — always `false` in practice). |
| `IsPlayerInspecting` | `UFUNCTION(BlueprintCallable) bool` | `bIsPlayerInspecting`. |
| `IsPlayerInvestigating` | `UFUNCTION(BlueprintCallable) bool` | `bIsPlayerInvestigating`. |
| `GetInteractableObject` | `TObjectPtr<UZWInteractionComponent>` (inline) | Highlighted (non-selected) interactable mirrored from the player component. |
| `SetInteractableObject` / `ResetInteractableObject` | `void` | Setter validates `IsValid`; reset un-highlights then clears. |
| `GetInteractedObject` | `TObjectPtr<UZWInteractionComponent>` (inline) | Oddity: **declared `TObjectPtr` in a header field context but the getter in the source actually reads from plain member `InteractedObject`; see private fields below.** |
| `SetInteractedObject` / `ResetInteractedObject` | `void` | Plain assign / clear (no un-highlight in subsystem's reset). |
| `GetInteractionSceneCapture` | `TObjectPtr<AZWInteractionSceneCapture>` (inline) | The active inspection stage actor (null if not spawned). |
| `SpawnInteractionSceneCapture` | `void` | Spawns `AZWInteractionSceneCapture` at `(0,0,-2000)`, only if none valid. |
| `DestroyInteractionSceneCapture` | `void` | `Destroy()` + clear. **No validity check** — crashes if never spawned. |
| `SetCamera` | `void SetCamera(AActor* NewTarget)` | See below. |
| `GetCameraLocation` / `GetCameraRotation` | `FVector / FRotator` | Delegated to the cached `UCameraComponent` on the active view target; `FVector::ZeroVector` / `FRotator::ZeroRotator` when none. |
| `UpdateCameraRotation` | `void UpdateCameraRotation(FVector2D LookAxisVector)` | Mouse-look accumulation for investigation cameras; see below. |
| `GetInitialCameraRotation` | `UFUNCTION(BlueprintCallable) FRotator` | Rotation the camera had when first attached during investigation. |
| `GetOffsetRotation` | `UFUNCTION(BlueprintCallable) FRotator` | Current pitch/yaw offset this session. |
| `StartInspection` (no params) | `UFUNCTION(BlueprintCallable, Category="Interaction System")` | Convenience overload: clones `InteractableObject`'s owner and starts inspecting it. |
| `StartInspection(UClass* ItemClass, const FActorSpawnParameters& InActorParams, bool IsRotatable)` | — | Full overload; see below. |
| `AdjustInspectionRotation` | `UFUNCTION(BlueprintCallable, Category="Interaction System") void AdjustInspectionRotation(FRotator Rotation)` | Forwards rotation delta to the scene capture's inspected actor. |
| `EndInspection` | `UFUNCTION(BlueprintCallable, Category="Interaction System") void EndInspection()` | Tears down inspection; see below. |
| `ResolveInvestigation` | `UFUNCTION(BlueprintCallable, Category="Interaction System") void ResolveInvestigation(AActor* NewTarget)` | Stack push/pop of investigation targets; see below. |
| `IsInvestigatedObject` | `bool IsInvestigatedObject(AActor* InObject)` | `true` if the actor has a `UZWInteractionComponent` that is inside `InvestigatedObjects`. |
| `StartInvestigation` | `UFUNCTION(BlueprintCallable, Category="Interaction System") void StartInvestigation(AActor* InitialTarget)` | Promotes `InteractableObject` → `InteractedObject`, seeds the stack, marks investigating, broadcasts `OnInvestigationStarted`. |
| `EndInvestigation` | `UFUNCTION(BlueprintCallable, Category="Interaction System") void EndInvestigation()` | Returns the view target to the player character, clears state, broadcasts `OnInvestigationEnded`. |

**Inspector / camera internals:**

- `SetCamera(AActor* NewTarget)`:
  1. Uses the world's **first** `APlayerController`; early-outs if controller or target invalid.
  2. `Controller->SetViewTargetWithBlend(NewTarget, 1, VTBlend_Linear)` — 1-second linear blend.
  3. Caches `UActorComponent* CameraComponent` on the new view target via `NewTarget->FindComponentByClass(UCameraComponent::StaticClass())`; if valid, records `InitialInvestigationCameraRotation = ActiveCamera->GetRelativeRotation()`, zeroes `CurrentRotationOffset`, stores `ActiveCameraRotation = InitialInvestigationCameraRotation`.
  4. `bIgnoreFirstInput = true` (drops the input burst that comes from the click that started the investigation).
  5. `OnInvestigationViewTargetChanged.Broadcast(NewTarget)`.
- `UpdateCameraRotation(FVector2D LookAxisVector)`:
  1. Swallows the first event (`bIgnoreFirstInput`).
  2. Adds `LookAxisVector.X` to offset yaw, `-= LookAxisVector.Y` to offset pitch, forces roll = 0, clamps pitch to `[-89, 89]` degrees.
  3. `ActiveCamera->SetRelativeRotation(InitialInvestigationCameraRotation + CurrentRotationOffset)`; stores in `ActiveCameraRotation`; emits three debug `UE_LOG(LogTemp, Log, …)` lines (initial rot, offset, new rot).
  4. Note the hard-coded `-89..89` yaw/pitch clamp — no config knob.
- `StartInspection()` (parameterless): promotes `GetInteractableObject()` to `InteractedObject`, resets interactable, and invokes the full overload with `ItemClass = Owner->GetClass()`, `FActorSpawnParameters{ Template = Owner }`, `IsRotatable = GetInteractedObject()->IsRotatable()`.
  - ⚠ `IsRotatable()` is not declared anywhere in this plugin's headers as shipped — see §8 (method missing on `UZWInteractionComponent`).
- `StartInspection(ItemClass, InActorParams, IsRotatable)`:
  1. Guard `if (bIsPlayerInspecting) return;`
  2. `World->SpawnActor<AActor>(ItemClass, InActorParams)` — template-spawned **clone** of the original, stored as `ClonedInspectedActor`.
  3. If the clone and scene capture both exist: forces `GetRootComponent()->SetMobility(Movable)`, then `InteractionSceneCapture->UpdateVisibility(ClonedInspectedActor)` (attaches the clone inside the capture stage and sets the capture's `ShowOnlyActorComponents`).
  4. If the clone exposes a `UArrowComponent`, calls `InteractionSceneCapture->SetLookAtRotation(FVector(0,0,0))`.
  5. `bIsPlayerInspecting = true`; `SetInputMode.ExecuteIfBound(Inspection)`; `StartInspectionDelegate.Broadcast(IsRotatable)`.
- `EndInspection()`:
  1. Guard `if (!bIsPlayerInspecting) return;`
  2. Demotes: `SetInteractableObject(GetInteractedObject())`. If investigation is still running **and** the stack has entries, re-attaches `InteractedObject` to `InvestigatedObjects.Last()`, otherwise resets both.
  3. `InteractionSceneCapture->UpdateVisibility(nullptr)` (detaches/clears show-only); destroys and clears `ClonedInspectedActor`.
  4. `bIsPlayerInspecting = false`.
  5. Input mode: if still investigating → `SetInputMode.ExecuteIfBound(Investigation)`; else `NoInteraction`.
  6. `EndInspectionDelegate.Broadcast()`.
  - Commented-out code references `UProjectXUIManager` and an `InteractionInspectionWidget`, suggesting pending integration with a UI-side manager (see §8).
- Investigation stack (`ResolveInvestigation` / `StartInvestigation` / `EndInvestigation` / `IsInvestigatedObject`):
  - Entry path A (`StartInvestigation`): promotes `InteractableObject` to `InteractedObject`, `InvestigatedObjects.AddUnique(...)`, then resets the interactable, `bIsPlayerInvestigating = true`, `OnInvestigationStarted.Broadcast(InitialTarget)`. (Also `SetInputMode.ExecuteIfBound(Investigation)` is commented out here.)
  - Entry path B (`ResolveInvestigation` with a valid target while not yet investigating): calls `StartInvestigation(NewTarget)` then `SetCamera(NewTarget)`.
  - `ResolveInvestigation` with a valid target while already investigating: appends via `InvestigatedObjects.AddUnique(GetInteractedObject())`, sets camera to the new target.
  - `ResolveInvestigation` with **invalid** target: if only one stacked item, calls `EndInvestigation()`, otherwise pops the current entry (`InvestigatedObjects.Remove(InteractedObject)`), selects the last remaining, and `SetCamera(InteractedObject->GetOwner())`.
  - `EndInvestigation()`: `SetCamera(PC->GetCharacter())` (blend back to player), `bIsPlayerInvestigating = false`, `InvestigatedObjects.Empty()`, `ResetInteractableObject()`, `ResetInteractedObject()`, then `OnInvestigationEnded.Broadcast()`.

**Private state:**
| Field | Type | Default |
|---|---|---|
| `ClonedInspectedActor` | `UPROPERTY() TObjectPtr<AActor>` | — |
| `InitialInvestigationCameraRotation` | `FRotator` | — |
| `CurrentRotationOffset` | `FRotator` | — |
| `bIgnoreFirstInput` | `bool` | `false` |
| `bIsPlayerInteracting` | `bool` | `false` |
| `bIsPlayerInvestigating` | `bool` | `false` |
| `bIsPlayerInspecting` | `bool` | `false` |
| `InteractableObject` | `TObjectPtr<UZWInteractionComponent>` | `nullptr` |
| `InteractedObject` | `TObjectPtr<UZWInteractionComponent>` | `nullptr` |
| `InteractionSceneCapture` | `TObjectPtr<AZWInteractionSceneCapture>` | — |
| `InvestigatedObjects` | `TArray<TObjectPtr<UZWInteractionComponent>>` | — |
| `ActiveCamera` | `USceneComponent*` | — |
| `ParentCamera` | `AActor*` (TODO comment: "I need to pass the information about the parent camera to the subsystem somehow") | `nullptr` |
| `ActiveCameraRotation` | `FRotator` | — |

### 4.6 `AZWInteractionSceneCapture` — `ZWInteractionSceneCapture.h/.cpp`
- **Base:** `ASceneCapture2D`. `UCLASS()` — **not** `BlueprintSpawnableComponent` and **not** exported to spawning from level/blueprints (default ctor is `protected`? — see note below); `UCLASS()` with no meta.
- Declaration quirk: in the header, the constructor is declared **without** `public:` in front of it, and subsequent members are separated by a `public:` block… actually the layout is: `GENERATED_BODY() AZWInteractionSceneCapture(const FObjectInitializer&)` at the **top of the class body (implicitly `private` for class unless `public:` appears before it — it does not in this file, so the ctor is in the `UPROPERTY`-visible private region). Subsequent `Setup*` member functions and public API are inside a separate `public:` section further down. **Result: the C++ constructor is private-ish, AAAm жесток — you cannot construct this actor from arbitrary C++ (only via `UObject` default construction paths/internal constructors); spawning it externally requires the `SpawnActor` path used by the subsystem.**
  - ⚠ Verbatim source: `GENERATED_BODY()` then `AZWInteractionSceneCapture(const FObjectInitializer& ObjectInitializer);` with no `public:` in between; the U Actors are created by `SpawnInteractionSceneCapture()` and this is presumably intentional preview-actor privacy. Flagged as a risk in §8.

**Component setup (ctor):**
| Component | SubName | Attach | Setup |
|---|---|---|---|
| `ItemComponent` (comment: "Item component.") — named `InspectedActor` in code | `UStaticMeshComponent` | `SetupAttachment(GetCaptureComponent2D())` | `RelativeLocation (50,0,0)`, `RelativeRotation (0,90,90)` — spawn point where inspected actors are attached. |
| `PostProcessComponent` | `UPostProcessComponent` | `SetupAttachment(RootComponent)` | See `SetupPostProcessComponent`. |
| `CubeComponent` | `UStaticMeshComponent` | `SetupAttachment(RootComponent)` | See `SetupCubeComponent`. |
| `RectLightComponent` | `URectLightComponent` | `SetupAttachment(RootComponent)` | See `SetupRectLightComponent`. |
| `GetCaptureComponent2D()` (inherited) | — | — | `TextureTarget` is loaded from **`/Game/Blueprints/InteractionSystem/RT_InspectionViewRenderTarget.RT_InspectionViewRenderTarget`** via `ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D>` (plugin content dependency, initialized before `Super::BeginPlay`). |
| `SetActorRotation(FRotator(0, 90, 90))` (on the scene actor itself) | — | — | |

**Helper methods (non-UFUNCTION, private/non-public as declared):**
| Method | Purpose |
|---|---|
| `SetupPostProcessComponent()` | `PostProcessComponent->SetRelativeLocation(FVector(-163,0,0))`; `Settings.WhiteTemp = 9416` (studio/white-balance grade for the inspection render). |
| `SetupCubeComponent()` | Loads `/Engine/BasicShapes/Cube.Cube` and `/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial` via `ConstructorHelpers`; sets as the mesh of `CubeComponent`, `RelativeScale3D = (5.8125, 5.8125, 5.8125)`, material on slot 0 — a backdrop cube. |
| `SetupRectLightComponent()` | `SetRelativeLocation(FVector(-236,0,0))`, `Intensity = 10000`, `IntensityUnits = ELightUnits::Unitless`, `SourceWidth = 496`, `SourceHeight = 456` — key light. |
| `SetupInspectedActor(AActor* InputActor)` | See below. |
| `UpdateVisibility(AActor* InActor)` | Thin wrapper around `SetupInpectedActor` (empty `if (InActor != nullptr)` block — placeholder). |
| `AddInspectedActorLocation(FVector Location)` X-only nudge | Clamps relative `X` to `[-20, 35]` — i.e. prevents pushing the item out of the capture frame forwards/backwards; adds the delta and calls `SetActorRelativeLocation` on the root of `InspectedActor`. **No null-check on `InspectedActor` or its root — possible crash.** |
| `AddInspectedActorRotation(FRotator Rotation)` | `AddActorLocalRotation(Rotation)` on `InspectedActor` if valid; warns `"InspectedActor not found!"` otherwise. |
| `SetLookAtRotation(FVector Location)` | `InspectedActor->SetActorRelativeRotation(UKismetMathLibrary::MakeRotFromX(Location))`; carries an inline `@TODO: Set correct rotation so the objects are displayed properly when inspected regardless of the pivot`. No null-check. |

**`SetupInspectedActor(AActor* InputActor)` exact behavior:**
1. If a previous `InspectedActor` was valid: `DetachFromActor(FDetachmentTransformRules::KeepWorldTransform)` on it, then `GetCaptureComponent2D()->ShowOnlyActorComponents(nullptr, true)` (empty the show-only set).
2. Set `InspectedActor = InputActor`, then `GetCaptureComponent2D()->ShowOnlyActorComponents(InspectedActor, true)` — only this actor renders in the capture.
3. If `InputActor` is valid: `InputActor->AttachToComponent(ItemComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None)`.

### 4.7 `UZWInteractionSystemSettings` — `ZWInteractionSystemSettings.h/.cpp`
- **Base:** `UDeveloperSettings`. `UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Interaction System Settings"))` — appears in **Project Settings → (category "ZW" via `GetCategoryName()` override) → ZW Interaction Settings** (`GetSectionText()` returns `INVTEXT("ZW Interaction Settings")`).
- .cpp contains only the include (no custom logic).

| Member | Signature | Purpose |
|---|---|---|
| `InteractionCollisionChannel` | `UPROPERTY(Config, EditAnywhere, Category="Interaction System", DisplayName="Interaction Collision Channel") TEnumAsByte<ECollisionChannel>` | The trace channel used for detection. The whole system reads this via `GetDefault<UZWInteractionSystemSettings>()` in `ZWInteractionComponent` (collision-response setup) and `ZWInteractionPlayerComponent` (trace conversion). |
| `GetCategoryName` | `virtual FName GetCategoryName() const override { return FName("ZW"); }` (`#if WITH_EDITORONLY_DATA`) | Editor category. |
| `GetSectionText` | `virtual FText GetSectionText() const override { return INVTEXT("ZW Interaction Settings"); }` (`#if WITH_EDITORONLY_DATA`) | Section title. |

### 4.8 `UZWInteractionSystem_Settings` — `ZWInteractionSystem_Settings.h/.cpp`
- **Base:** plain `UObject`. `UCLASS(Config=Game, DefaultConfig, meta=(DisplayName="Interaction System Settings"))` — a barebones config class (no `UDeveloperSettings` base, so it does **not** show in Project Settings; presumably edited via `ini` or reflected editing elsewhere).
- Same `DisplayName` as the DeveloperSettings class → metadata collision ("Interaction System Settings" for both), worth flagging.

| Member | Signature | Purpose |
|---|---|---|
| `InvestigationMappingContext` | `UPROPERTY(Config, EditAnywhere, Category="Interaction System", DisplayName="Investigation Input Mapping Context") TSoftObjectPtr<UInputMappingContext>` | Soft-ref to the Enhanced Input Mapping Context the investigation mode should push (e.g. mouse-look controls). |
| .cpp | — | contains only the `#include` (empty body construct, nowhere bound — see §8). |

## 5. Implementacja (Private) — w tym scene capture / trace

### Module bootstrap (`ZWInteraction.cpp`)
`FZWInteractionModule::StartupModule()` and `ShutdownModule()` are empty — no subsystem registration, no console variables, no asset type registration. Everything relies on UCLASS auto-registration (`IMPLEMENT_MODULE` + `GENERATED_BODY`).

### System-level implementatations (`ZWInteractionSubsystem.cpp`)
Header includes: `CommonInputModeTypes.h`, `CommonInputSubsystem.h`, `EnhancedInputSubsystems.h`, `ZWInteractionSystem_Settings.h`, `Kismet/GameplayStatics.h`, `GameFramework/Character.h`, `Camera/CameraComponent.h`, `Components/ArrowComponent.h`, `Input/CommonUIActionRouterBase.h`, plus a commented-out `InteractionInspectionWidget.h`.
- As noted in §4.5, routine tasks handled here:
  - Interactable/Interacted bookkeeping (mirror of the player component's state, shared across modes).
  - Scene capture actor lifetime (spawn-once at fixed world position `(0,0,-2000)`, destroy).
  - Player controller view target management (a full one-second linear blend — inputs are intentionally ignored for the first event).
  - Camera orientation (`UCameraComponent` under the active view target) with pitch clamp and per-frame rotation delta.

### Player-side detection pipeline (`ZWInteractionPlayerComponent.cpp`)
Includes `ScreenPass.h` (implied engine screen utilities; in this file the trace path is via `UKismetSystemLibrary`), `ZWInteractionComponent.h`, `ZWInteractionSystemSettings.h`, `Kismet/KismetSystemLibrary.h`, `Kismet/KismetMathLibrary.h`.

**Exact per-tick flow:**
```
TickComponent
  └─ DetectInteractiveObjects()
       ├─ ActorsToIgnore = [ GetOwner() ]
       ├─ ResolveLineTracePoints(out TraceStart, out TraceEnd, out traceRadius)
       │     └─ if owner's InstigatorController is an APlayerController:
       │           deproject ViewportCenter → DetectorLocation/Direction (overrides the component's own ones)
       ├─ ResolveLineTraceIgnoredActors (no-op default)
       ├─ Settings = GetDefault<UZWInteractionSystemSettings>(); return silently if null
       ├─ CollisionTrace = ConvertToTraceType(Settings->InteractionCollisionChannel)
       ├─ DebugTrace = bDrawDebugTrace ? ForDuration : None
       ├─ UKismetSystemLibrary::SphereTraceSingle(
       │     World, TraceStart, TraceEnd, DetectionTraceRadius,
       │     CollisionTrace, false /*bTraceComplex*/,
       │     ActorsToIgnore, DebugTrace, HitResult, true /*bIgnoreSelf*/)
       ├─ Hit  → ResolveInteractiveObject(hit actor)
       └─ Miss & previous present → ResetInteractableObject()
```

`ResolveInteractiveObject(AActor*)`:
1. `ResetInteractableObject()` — un-highlight the previous.
2. Early return if `IsValid(GetInteractedObject())` and its `GetOwner()` equals the just-hit actor (already-engaged object stays un-highlighted).
3. `GetComponentByClass<UZWInteractionComponent>` on the hit actor; skip if null; **`IsActive()` is checked before adopting** the component; then `SetInteractableObject` + `ToggleHighlight(true)`.
4. If the lookup returned null (e.g. actor without a component), re-run `ResetInteractableObject()`.

`Interact()`:
- Operates on `GetInteractableObject()` (the currently highlighted one) **only**; guarded additionally by `InteractionComponent->IsHighlighted()` — i.e., you cannot interact with an object that is not currently visually highlighted.

### Scene capture stage (`ZWInteractionSceneCapture.cpp`)
Includes `Components/SceneCaptureComponent2D.h`, `Components/PostProcessComponent.h`, `Components/RectLightComponent.h`, `Engine/TextureRenderTarget2D.h`, `Kismet/KismetMathLibrary.h`.

- Construction (as tabulated in §4.6): one `UStaticMeshComponent` "InspectedActor" (misleadingly named — the *actor* pointer member is separate), one `UPostProcessComponent`, one `UStaticMeshComponent` "CubeComponent", one `URectLightComponent`; a `ConstructorHelpers::FObjectFinder` pins the capture target to `/Game/Blueprints/InteractionSystem/RT_InspectionViewRenderTarget.RT_InspectionViewRenderTarget`.
- Hot-reload caveat: `ConstructorHelpers::FObjectFinder` in the constructor is load-time only; the render-target asset must exist in the plugin content, otherwise `GetCaptureComponent2D()->TextureTarget = TextureRenderTarget2D` writes `nullptr` (silent failure — capture produces nothing).
- The empty `if (InActor != nullptr) { }` body inside `UpdateVisibility` is leftover scaffolding.
- Stack traces: `AddInspectedActorLocation` (X-clamp `[−20, 35]`), `AddInspectedActorRotation` (with a `UE_LOG` warning on missing inspected actor), `SetLookAtRotation` (uses `MakeRotFromX`).

## 6. Konfiguracja (.ini)

File: `Config/DefaultZWInteraction.ini`

**`[CoreRedirects]`** — class renames from earlier (generic) names to the current `ZW`-prefixed names, preserving previously saved assets:
```
+ClassRedirects=(OldName="/Script/ZWInteraction.InteractionComponent",NewName="/Script/ZWInteraction.ZWInteractionComponent")
+ClassRedirects=(OldName="/Script/ZWInteraction.InteractionSystem_Settings",NewName="/Script/ZWInteraction.ZWInteractionSystem_Settings")
+ClassRedirects=(OldName="/Script/ZWInteraction.InteractionPlayerComponent",NewName="/Script/ZWInteraction.ZWInteractionPlayerComponent")
```
These match the three formerly-`Interaction*` actor components/classes that the project refactored under a `ZW` prefix. No other ini keys, no default values for `InteractionCollisionChannel` or `InvestigationMappingContext` — those come from the project's own `DefaultGame.ini`/`DefaultZWInteraction.ini` when the settings objects are saved.

Runtime config objects:
- `UZWInteractionSystemSettings` (`Config=Game, defaultconfig`) — its property `InteractionCollisionChannel` is written to the project's `DefaultGame.ini` on save from **Project Settings → ZW → ZW Interaction Settings → Interaction Collision Channel**.
- `UZWInteractionSystem_Settings` (`Config=Game, DefaultConfig`) — property `InvestigationMappingContext` is a `TSoftObjectPtr<UInputMappingContext>`, saved likewise.

## 7. Zależności wewnątrz ZWSuite

Direct in-plugin coupling:
- `ZWInteractionComponent` ↔ `ZWInteractionSystemSettings` (channel-based collision response setup in `BeginPlay`).
- `ZWInteractionPlayerComponent` ↔ `ZWInteractionComponent` (uses it as the interactable handle) and ↔ `ZWInteractionSystemSettings` (trace channel).
- `ZWInteractionSubsystem` ↔ `ZWInteractionComponent` (highlight/interact side effects, `IsRotatable()` probe), ↔ `ZWInteractionSceneCapture` (inspection stage lifetime and show-only list), ↔ `ZWInteractionSystem_Settings` (commented-out / context assignments — currently no effective code reference; see §8).

Cross-plugin references visible in comments:
- `//InteractionInspectionWidget.h` (include commented out) and an `//InspectionWidget->InspectionWidgetEndInspectionDelegate.Unbind();` comment inside `EndInspection`.
- A commented-out block that would have used `UProjectXUIManager` (`GetWorld()->GetFirstLocalPlayerFromController()->GetSubsystem<UProjectXUIManager>()` + `RequestGameInputMode()`), establishing that ZWSuite has a UI-manager plugin/layer intended to control input modes in concert with this subsystem.

No other ZWSuite plugin paths, includes, or module names appear in the source — i.e., ZWInteraction is currently **self-contained** apart from these commented-out hooks (no explicit cross-target dependency on other ZW modules of this suite).

External engine plugin dependencies: `CommonUI` / `CommonInput` (via .uplugin + Build.cs + subsystem includes — only partially exercised, see §8), `EnhancedInput` (input mapping context asset in the settings class), and standard `Engine`/`Slate`.

## 8. Uwagi / ryzyka

1. **`IZWInteractionInterface` is bypassed** — the header literally marks "BYPASSED FOR NOW". Its `UFUNCTION(BlueprintCallable, BlueprintNativeEvent)` `BPToggleHighlight` sits on a non-UCLASS base (plain generated interface class) and no vtable dispatch is wired from `UZWInteractionComponent`; anything still referencing `Inspect()`/`Investigate()` semantics has no active caller inside the plugin.
2. **`UZWInteractionComponent::IsRotatable()` is called but not declared** (`UZWInteractionSubsystem::StartInspection()` uses `GetInteractedObject()->IsRotatable()`, and `UZWInteractionSceneCapture` has no such call path). The supplied `ZWInteractionComponent.h` has no `IsRotatable` member. Either the method is expected to live in an upstream/derived class in ZWSuite, or the reconstructed source is incomplete → **compile error risk** on a clean build.
3. **`UZWInteractionSceneCapture` ctor visibility:** in the reconstructed header the constructor `AZWInteractionSceneCapture(const FObjectInitializer&)` appears before any `public:` keyword, making it private in a `UCLASS()`-declared C++ class — combined with `SpawnActor<AZWInteractionSceneCapture>` from the subsystem this is fragile; if intentional, it's unusual (actors normally expose a public ctor for the factory).
4. **`DestroyInteractionSceneCapture()` has no null guard** — destroys unconditionally. If the capture was never spawned or already destroyed, this crashes.
5. **`AddInspectedActorLocation` / `SetLookAtRotation` have no null guards on `InspectedActor` / `GetRootComponent()`** and `AddInspectedActorLocation` clamps only `X` in `[−20, 35]` — magic numbers with no editor-exposed tunables.
6. **`FSetInputModeDelegate`, the investigation mapping context, `CollectiveInputMode`** — none of the Enhanced Input plumbing actually applies `UZWInteractionSystem_Settings::InvestigationMappingContext` anywhere in this snapshot (no `AddMappingContext` call on `EnhancedInputSubsystems` even though the header `EnhancedInputSubsystems.h` is included). Input-mode switching is entirely the responsibility of whoever binds `SetInputMode`, and the inspection camera never swaps in the mapping context automatically.
7. **Two config classes share the display name "Interaction System Settings"** — `UZWInteractionSystemSettings` (DeveloperSettings) and `UZWInteractionSystem_Settings` (plain `UObject`). In the Projects Settings UI only the former appears (category **ZW**); the latter is invisible, orphaned config — only editable via solo-class reflection editors or `DefaultGame.ini` hand edit.
8. **Duplicate DisplayName / context handling** — `UZWInteractionSystem_Settings::InvestigationMappingContext` config is declared but never read by the plugin itself; dead config surface.
9. **`bIsPlayerInteracting` has no writer** in this snapshot — `IsPlayerInteracting()` always reads `false`. Dead alongside the bypassed interface.
10. **Debug spam** — `UpdateCameraRotation` unconditionally prints three `UE_LOG(LogTemp, Log, ...)` blocks per frame of mouse movement. No verbosity gating.
11. **`Cast<UStaticMeshComponent>`/`Cast<USkeletalMeshComponent>` fallbacks** — the `FindComponentByTag(UStaticMeshComponent::StaticClass(), FName("MainInteractionMesh"))` path only kicks in if the first `GetComponentByClass` finds **no** component; an actor with multiple mesh components will silently attach the *first* one regardless of the `MainInteractionMesh` tag — the tag-based selection can never win against a first non-tagged mesh. Worth verifying in a project with more than one mesh per interactable.
12. **Asset dependency**: `/Game/Blueprints/InteractionSystem/RT_InspectionViewRenderTarget` must exist in the project/plugin content; the `FObjectFinder` in the actor constructor silently no-ops (and `TextureTarget` becomes null) if missing → capture yields blank frames.
13. **Cross-plugin coupling is commented out**: `UProjectXUIManager` usage and `InteractionInspectionWidget` bindings are present only in comments. If they were real features, they are currently disconnected; `.uplugin`/Build.cs don't reference any other ZW module to make them compile anyway.
14. **`ParentCamera` field** is reserved with an explicit `//TODO`; camera restore is done via `PC->GetCharacter()` rather than by remembering the parent camera, so any non-character-parented camera (e.g. a vehicle/cinematic rig) will be lost after `EndInvestigation()`.
15. **`UZWInteractionComponent::ToggleHighlight` behavior**: it passes `IsHighlighted` straight through rather than using the local `bShouldBeHighlighted` flag it computes — the flag mirrors the input rather than the "mesh present" logic, which reads oddly but is harmless in practice.
16. **`CommonUI` subscriptions** — `#include "Input/CommonUIActionRouterBase.h"` and `CommonInputModeTypes.h` are listed but not used in any surviving code path; the CommonUI-related `.uplugin` dependency might be removable once cleanup happens, or the missing pieces are the commented-out UI-manager integration described above.

*Documentation generated from plugin sources as provided; no fabricated content — items marked "brak"/none reflect genuinely absent declarations.*

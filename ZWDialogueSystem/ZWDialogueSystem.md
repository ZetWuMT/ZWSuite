# ZWDialogueSystem — Technical Documentation

> Source reviewed: `/opt/data/projectx/ZWSuite-src/ZWDialogueSystem` (every `.uplugin`, `.Build.cs`, `Config/*.ini`, and every `Public/*.h` + `Private/*.cpp` under `Source/`, including the nested standalone plugin `ZWMovieSceneDialogueTrack/`).

---

## 1. Overview

**ZWDialogueSystem** is the largest plugin in the ZWSuite collection. It is a **composite (nested) plugin**: the top-level `ZWDialogueSystem.uplugin` declares 4 modules, and a directory `ZWMovieSceneDialogueTrack/` physically sits inside it as a **separate, self-contained sub-plugin** with its own `.uplugin`, its own two modules, and prebuilt Win64 binaries (`Binaries/Win64/UnrealEditor-ZWMovieSceneDialogueTrack.dll`, `UnrealEditor-ZWMovieSceneDialogueTrackEditor.dll` + PDBs, `Resources/Icon128.png`).

The plugin provides two cooperating halves:

1. **Dialogue driving core** (`ZWDialogueSystem` runtime module) — a `GameInstanceSubsystem`-based dialogue bus: Sequencer (MovieScene) dialogue sections trigger dialogue lines via a token object (`FZWDialogueToken`), the subsystem notifies registered `IZWDialogueLineHandler` implementors (UI widgets, choice subsystems...) ordered by `GetOrder()`, and integrates `RuntimeAudioImporter` to asynchronously import per-line generated WAV files (`FZWDialogueAudioData.AudioGuid` → `Content/Localization/Audio/<lang>/<guid>.wav`) so lines can play while the Sequencer keeps playing.
2. **Dialogue choice system** (`ZWDialogueSystem` + CommonUI) — `UZWDialogueChoiceSubsystem` tracks chosen options per node GUID (`ChosenOptions` TMultiMap), exposes a `FOnChoiceStartedDelegate`, and drives a CommonUI panel (`UZWDialogueChoicePanelWidget`) that builds choice widgets from `UZWDialogueChoicePanelWidgetData` (MainChoices + Choices), supports controller/gamepad navigation via CommonUI UI Action bindings, and marks the choice object dirty on confirm.
3. **Editor-side audio generation** (`ZWDialogueSystemEditor`) — `FZWDialogueAudioGenerator` calls **Google Cloud Text-to-Speech** (`https://texttospeech.googleapis.com/v1/text:synthesize`), decodes the base64 `audioContent` to a WAV, parses the WAV header (`ByteRate` at offset 28) to fill `PrecalculatedDuration`, and saves to `Content/Localization/Audio/<LangCode>/<Guid>.wav`. Settings come from `UZWDialogueSettings` (a `UDeveloperSettings` `Config=Game` class).
4. **Sequencer integration** (`ZWMovieSceneDialogueTrack` + its editor module) — a custom `UMovieSceneNameableTrack` (`UZWMovieSceneDialogueTrack`) / `UMovieSceneSection` (`UZWMovieSceneDialogueSection`) pair with a full eval-template pipeline (`FZWMovieSceneDialogueSectionTemplate` → `FZWMovieSceneDialogueExecutionToken`) that triggers/ticks/closes dialogue lines during LevelSequence playback, plus a Sequencer track editor (`FZWDialogueTrackEditor`, `FZWDialogueSection`) for authoring.

---

## 2. Metadata (.uplugin files)

### 2.1 Outer plugin: `ZWDialogueSystem.uplugin`

| Field | Value |
|---|---|
| FileVersion / Version / VersionName | 3 / 1 / 1.0 |
| FriendlyName | ZWDialogueSystem |
| Description / CreatedBy / DocsURL / MarketplaceURL / SupportURL / CreatedByURL | (empty) |
| Category | Other |
| CanContainContent | true |
| IsBetaVersion / IsExperimentalVersion / Installed | false / false / false |

**Modules (4):**

| # | Module Name | Type | LoadingPhase |
|---|---|---|---|
| 1 | ZWMovieSceneDialogueTrack | Runtime | PreDefault |
| 2 | ZWMovieSceneDialogueTrackEditor | Editor | Default |
| 3 | ZWDialogueSystem | Runtime | Default |
| 4 | ZWDialogueSystemEditor | Editor | Default |

**Plugins dependency list:**

| Plugin | Enabled |
|---|---|
| AssetSearch | true |
| EditorScriptingUtilities | true |
| CommonUI | true |
| RuntimeAudioImporter | true |

Note: the plugin source directory contains **no `Source/ZWMovieSceneDialogueTrack(...)` folders of its own** — the module sources for modules 1–2 live **inside the nested sub-plugin directory** `ZWMovieSceneDialogueTrack/Source/...`. Only `ZWDialogueSystem` and `ZWDialogueSystemEditor` have sources directly under `Source/`. The outer `.uplugin` therefore references modules whose code ships in the embedded sub-plugin.

### 2.2 Nested sub-plugin: `ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.uplugin`

| Field | Value |
|---|---|
| FileVersion / Version / VersionName | 3 / 1 / 1.0 |
| FriendlyName | ZWMovieSceneDialogueTrack |
| Description etc. | (empty) |
| Category | Other |
| CanContainContent | true |

**Modules (2):**

| # | Module Name | Type | LoadingPhase |
|---|---|---|---|
| 1 | ZWMovieSceneDialogueTrack | Runtime | PreDefault |
| 2 | ZWMovieSceneDialogueTrackEditor | Editor | Default |

**Plugins dependency list:** AssetSearch (true), EditorScriptingUtilities (true), CommonUI (true). **No** `RuntimeAudioImporter` reference (that is exclusive to the outer plugin).

Ships prebuilt binaries: `Binaries/Win64/UnrealEditor-ZWMovieSceneDialogueTrack.{dll,pdb}`, `UnrealEditor-ZWMovieSceneDialogueTrackEditor.{dll,pdb}`, `UnrealEditor.modules`, `Resources/Icon128.png`.

### 2.3 Config: `Config/DefaultZWMovieSceneDialogueTrack.ini`

Contains only a bare `[CoreRedirects]` section — no actual redirects are defined (effectively empty).

---

## 3. Sub-modules (Build.cs dependency map)

### 3.1 `Source/ZWDialogueSystem/ZWDialogueSystem.Build.cs`

- PCHUsage: `UseExplicitOrSharedPCHs`
- **Public deps:** `Core`, `CommonUI`, `GameplayTags`, `DeveloperSettings`, `RuntimeAudioImporter`
- **Private deps:** `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `UMG`
- **DynamicallyLoaded:** (none)

### 3.2 `Source/ZWDialogueSystemEditor/ZWDialogueSystemEditor.Build.cs`

- PCHUsage: `UseExplicitOrSharedPCHs`
- **Public deps:** `Core`, `ZWDialogueSystem`, `BlueprintEditorLibrary`
- **Private deps:** `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `HTTP`, `Json`
- **DynamicallyLoaded:** (none)

### 3.3 Nested sub-plugin — outer copy of `ZWMovieSceneDialogueTrack.Build.cs`

**File:** `Source/ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.Build.cs` (this file exists at the *outer plugin level*, despite no `Source/ZWMovieSceneDialogueTrack` sources — it duplicates the sub-plugin's Build.cs with one addition).

- PCHUsage: `UseExplicitOrSharedPCHs`
- **Public deps:** `LevelSequence`, `UMG`, `CommonUI`, **`ZWDialogueSystem`** ← this copy links against the parent plugin's runtime module
- **Private deps:** `Core`, `CoreUObject`, `DeveloperSettings`, `Engine`, `GameplayTags`, `MovieScene`, `MovieSceneTracks`, `Slate`, `SlateCore`
- Editor-only (guarded `if (Target.Type == TargetType.Editor)`): `PublicDependencyModuleNames` += `MessageLog`, `UnrealEd`
- **DynamicallyLoaded:** (none)

### 3.4 Nested sub-plugin — module `ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.Source/ZWMovieSceneDialogueTrack.Build.cs`

**File:** `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.Build.cs`

- **Public deps:** `LevelSequence`, `UMG`, `CommonUI` *(does NOT depend on `ZWDialogueSystem` — the sub-plugin is source-independent from its parent)*
- **Private deps:** `Core`, `CoreUObject`, `DeveloperSettings`, `Engine`, `GameplayTags`, `MovieScene`, `MovieSceneTracks`, `Slate`, `SlateCore`
- Editor-target only: += `MessageLog`, `UnrealEd` (public)
- **DynamicallyLoaded:** (none)

### 3.5 Nested sub-plugin — module `ZWMovieSceneDialogueTrackEditor`

**File:** `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrackEditor/ZWMovieSceneDialogueTrackEditor.Build.cs`

- PCHUsage: `UseExplicitOrSharedPCHs`
- **Public deps:** `EditorSubsystem`, `ZWMovieSceneDialogueTrack`, `MessageLog`
- **Private deps (34 modules):** `ApplicationCore`, `AssetSearch`, `AssetTools`, `BlueprintGraph`, `ClassViewer`, `ContentBrowser`, `Core`, `CoreUObject`, `DetailCustomizations`, `DeveloperSettings`, `EditorFramework`, `EditorScriptingUtilities`, `EditorStyle`, `Engine`, `GraphEditor`, `InputCore`, `Json`, `JsonUtilities`, `Kismet`, `KismetWidgets`, `LevelEditor`, `LevelSequence`, `MovieScene`, `MovieSceneTools`, `MovieSceneTracks`, `Projects`, `PropertyEditor`, `PropertyPath`, `RenderCore`, `Sequencer`, `Slate`, `SlateCore`, `SourceControl`, `ToolMenus`, `UnrealEd`
- **DynamicallyLoaded:** (none)

---

## 4. Public API — classes

Type inventory used below: **UCLASS**, **USTRUCT**, **UENUM**, **UINTERFACE**, and plain C++ module classes. Everything below is verified against actual headers/impls; commented-out members are marked as such.

### 4.1 Module `ZWDialogueSystem` — Dialogue data model (`Source/ZWDialogueSystem/Public/`)

#### `FZWDialogueData` → `USTRUCT(BlueprintType)` (files: `ZWDialogueData.h/.cpp`)
Core per-line dialogue datum; passed through the whole pipeline.
- `FGuid EventID` — EditAnywhere; ctor (`FZWDialogueData.cpp`) initializes with `FGuid::NewGuid()`.
- `FName SpeakerID` — EditAnywhere, meta `GetOptions = "GetAvailableSpeakers"`; comment: FName identifier of the speaker, localizable version defined separately; requires `GetAvailableSpeakers` to be defined (declaration of that helper is not present in the reviewed code: "brak").
- `FText Speaker` — EditAnywhere.
- `FText DialogueLine` — EditAnywhere.
- `TWeakInterfacePtr<IZWDialogueLineHandler> FinalDialogueLineHandler = nullptr` — transient (non-UPROPERTY); the handler that returned `Final` when the line started.
- `FZWDialogueAudioData AudioData` — nested audio info.
- `operator==(const FZWDialogueData&, const FGuid&)` free overload exists so `FindByKey`/`IndexOfByKey` work on `EventID`.

#### `FZWDialogueAudioData` → `USTRUCT(BlueprintType)` (`ZWDialogueAudioData.h/.cpp`)
- `FGuid AudioGuid` — unique id, doubles as the `.wav` file name inside the Localization folder.
- `float PrecalculatedDuration = 0.0f` — useful in Sequencer so a section knows its length up-front instead of async-loading the file in the editor.

#### `FZWDialogueChoice` / `FZWChoiceData` → `USTRUCT()` ×2 (`ZWChoiceData.h/.cpp`) *— appears to be a legacy/duplicate of the choice-data model (identical shape to `FDialogueChoice`/`FChoiceData` in the track module).*
- `FZWDialogueChoice`: `FGuid Guid` (VisibleAnywhere, `NoResetToDefault`), ctor `FZWDialogueChoice(FGuid Guid)`, `FText ChoiceText`, `FName SocketName`, `bool bMainChoice = false`, `bool bSingleUse = false`; commented-out QoL members (`IconPreset` with `GetChoiceIconStrings`, `GetConditionsString(const UFlowNode*)`).
- `FZWChoiceData`: `TArray<FZWDialogueChoice> Choices` with `TitleProperty = "SocketName"` (note `EditFixedSize` commented out).

#### `UZWDialogueChoiceChangeableObject` → `UCLASS()` base `UObject` (`ZWDialogueChoiceChangeableObject.h/.cpp`)
Dirty-trackable data object base.
- `DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueChangedDelegate, UZWDialogueChoiceChangeableObject*, NewValue)`; `FOnValueChangedDelegate OnChange` (the `BlueprintAssignable` UPROPERTY is commented out).
- `UFUNCTION(BlueprintCallable) void SetDirty(bool bTriggerOnChange = false)` — sets `bIsDirty = true`, optionally broadcasts `OnChange`.
- `void ClearDirty()`.
- `void BroadcastValueChanged()` — broadcasts only while `bIsDirty` is true, then clears.
- State: `bool bIsDirty = false`.

#### `UZWDialogueChoiceData` → `UCLASS()` base `UZWDialogueChoiceChangeableObject` (`ZWDialogueChoiceData.h/.cpp`)
Single-choice payload used by the panel widgets.
- `FText ChoiceText`, `FName ChoiceLabel`, `FName IconPreset`, `bool bWasChosen = false`, `bool bSingleUse = false`, `bool bMainChoice = false` — all `EditDefaultsOnly, BlueprintReadWrite`.

#### `UZWDialogueChoicePanelWidgetData` → `UCLASS()` base `UZWDialogueChoiceChangeableObject` (`UI/ZWDialogueChoicePanelWidgetData.h/.cpp`)
View-model handed to the choice panel:
- `TArray<TObjectPtr<UZWDialogueChoiceData>> MainChoices`
- `TArray<TObjectPtr<UZWDialogueChoiceData>> Choices`
- `FName ConfirmedChoice`

### 4.2 Module `ZWDialogueSystem` — Dialogue-line handler pipeline

#### `EZWStartDialogueResult` → plain C++ `enum class` (NOT UENUM) (`ZWDialogueLineHandler.h`)
`Handled`, `Unhandled`, `Final`.

#### `UZWDialogueLineHandler` / `IZWDialogueLineHandler` → `UINTERFACE(Blueprintable)` (`ZWDialogueLineHandler.h/.cpp`)
The bus contract; all dialogue consumers implement it:
- `virtual uint32 GetOrder() const = 0`
- `virtual EZWStartDialogueResult OnStartDialogueLine(const FZWDialogueData& DialogueData) = 0`
- `virtual void OnFinishDialogueLine(const FZWDialogueData& DialogueData) = 0`
- `virtual void OnDialogueLineUpdated(const FZWDialogueData& DialogueData) {}` (default no-op)

#### `FZWDialogueSystemModule` → `IModuleInterface` (`ZWDialogueSystem.h/.cpp`, ` IMPLEMENT_MODULE`)
Empty `StartupModule` / `ShutdownModule`; LOCTEXT namespace `"FZWDialogueSystemModule"`.

#### `UZWMovieSceneDialogueSubsystem` → `UCLASS()` base `UGameInstanceSubsystem` (`ZWMovieSceneDialogueSubsystem.h/.cpp`)
The runtime dialogue hub (the most substantial runtime class). Public surface:
- Delegates (plain multicast, not UPROPERTY):
  - `FDialogueStartedEvent DialogueStartedEvent` — `DECLARE_MULTICAST_DELEGATE_OneParam(...) (`const FGuid`)` (declared; never broadcast in current code — TODO comment "Potential multi-casts for additional subsystems if needed")
  - `FDialogueEndedEvent DialogueEndedEvent` — `const FGuid` (declared, never broadcast)
  - `FDialogueTickEvent DialogueTickEvent` — `DECLARE_MULTICAST_DELEGATE_TwoParams(const FGuid, float)`; broadcast every `TickDialogue`.
- `virtual void Deinitialize() override` — empties `DialogueLineHandlers`.
- `void TickDialogue(FGuid EventID, float CurrentTime)` — early-out if `CurrentDialogueLines.IsEmpty()`, broadcasts `DialogueTickEvent(EventID, CurrentTime)`, locates the active line by EventID (`check` non-null), then walks handlers (ordered) invoking `OnDialogueLineUpdated`, terminating at `FinalDialogueLineHandler`.
- `FZWDialogueTokenPtr TriggerDialogueLine(const FZWDialogueDetails& DialogueLine)` overload — builds a fresh GUID (`FGuid::NewGuid()`), constructs `FZWDialogueData` from `SpeakerID`/`Speaker`/`DialogueText`, forwards to the main overload.
- `FZWDialogueTokenPtr TriggerDialogueLine(FZWDialogueData DialogueData)` — **the token-returning entry point used by MovieScene templates.** Behavior:
  1. `MakeShared<FZWDialogueToken>(this, DialogueData.EventID)` (token ready immediately — "Sequencer needs it at once").
  2. If `AudioData.AudioGuid.IsValid()`:
     - builds path `FPaths::ProjectContentDir() / "Localization/Audio" / <lang="en-gb"> / <Guid>.wav` (language is hardcoded `"en-gb"` here),
     - forces a byte load (`FFileHelper::LoadFileToArray`),
     - creates `URuntimeAudioImporterLibrary`, registers it in `ActiveImports` (UPROPERTY `TMap<FGuid, URuntimeAudioImporterLibrary*>`), seeds `PendingPlaybackTimes` (TMap<FGuid,float> = 0.0f) and dumps the line into `PendingAudioDialogues` (TMap<FGuid, FZWDialogueData>) — the "handoff map" consumed on import finish,
     - binds `OnProgressNative` → `OnAudioImportProgress`, `OnResultNative` → `OnAudioImportFinished`, calls `ImportAudioFromFile(FullPath, ERuntimeAudioFormat::Auto)`, sets `bIsAudioLoading = true`.
  3. If audio is *not* loading (no GUID or I/O failure), notifies handlers immediately so the UI is not blocked.
  4. Returns the token; the actual UI notification happens when import completes (`OnAudioImportFinished` → `NotifyHandlers(PendingData)`).
- `void RegisterDialogueHandler(IZWDialogueLineHandler* Handler)` — adds to `TArray<TWeakInterfacePtr<IZWDialogueLineHandler>> DialogueLineHandlers`, sorts by `GetOrder()`, `ensureMsgf(... "Orders need to be unique")` on equal orders.
- `void UnregisterDialogueHandler(IZWDialogueLineHandler* Handler)` — `RemoveSingle`.
- Private helpers: `CloseDialogueLine(const FGuid& EventID)` (notifies `OnFinishDialogueLine` up to `FinalDialogueLineHandler`, removes the line from `CurrentDialogueLines`), `NotifyHandlers(FZWDialogueData)` (walks handlers calling `OnStartDialogueLine`, first `Final` result becomes `FinalDialogueLineHandler` and ends the loop; appends to `CurrentDialogueLines`), `OnAudioImportProgress(int32)` (log-only), `OnAudioImportFinished(URuntimeAudioImporterLibrary* Importer, UImportedSoundWave*, ERuntimeImportStatus Result)`.
- `OnAudioImportFinished` details: resolves `EventID` from `ActiveImports.FindKey(Importer)` (early return otherwise *— note: `ActiveImports.Remove(Importer)` at this point is commented out*); pulls `PendingData` from `PendingAudioDialogues` (early return if missing); on `SuccessfulImport` spawns 2D audio via `UGameplayStatics::SpawnSound2D(GetWorld(), ImportedSoundWave, 1.0f, 1.0f, 0.0f, nullptr, false, bAutoDestroy=true)`, then `AudioComp->Play(TargetStartTime)` using the Sequencer back-logged time from `PendingPlaybackTimes` (removes `ActiveImports` entry after); on failure logs `"Błąd dekodowania w wątku! EventID: %s"`; removes `PendingPlaybackTimes`; finally notifies handlers with the pending data once audio is (or failed to be) playing.
- Storage: `TArray<FZWDialogueData> CurrentDialogueLines`, `TMap<FGuid, FZWDialogueData> PendingAudioDialogues`, `TMap<FGuid, URuntimeAudioImporterLibrary*> ActiveImports`, `TMap<FGuid, float> PendingPlaybackTimes`.
- `friend FZWDialogueToken` — the token's destructor calls `DialogueSubsystem->CloseDialogueLine(EventID)`; dropping the shared token closes the line.

Supporting plain classes in the same header:
- `FZWDialogueToken` — `FGuid EventID`, `TWeakObjectPtr<UZWMovieSceneDialogueSubsystem> DialogueSubsystemWeak`; dtor closes the dialogue line.
- `FZWDialogueDetails` — `FName SpeakerID`, `FText Speaker`, `FText DialogueText`, `float Progress /* In Seconds */ ` (the Details flavor passed by editor-side tooling; `Progress` is distinct from `PrecalculatedDuration`).

### 4.3 Module `ZWDialogueSystem` — Choice subsystem

#### `UZWDialogueChoiceSubsystem` → `UCLASS()` base `UGameInstanceSubsystem`, implements `IZWDialogueLineHandler` (`ZWDialogueChoiceSubsystem.h/.cpp`)
- `DECLARE_MULTICAST_DELEGATE_OneParam(FOnChoiceStartedDelegate, const TObjectPtr<UZWDialogueChoicePanelWidgetData>&)`; `FOnChoiceStartedDelegate ChoiceStarted`.
- `virtual void Initialize(FSubsystemCollectionBase& Collection)` — `Collection.InitializeDependency<UZWMovieSceneDialogueSubsystem>()` then `RegisterDialogueHandler(this)` (handler order 11).
- `uint32 GetOrder() const` → `11u`.
- `OnStartDialogueLine` — stores `LastDialogueLine = DialogueData`, returns `Handled` (so the propagate chain stops flowing after widget handler at order 150 if widget returns Final; 11 < 150 so choice sub runs first).
- `OnFinishDialogueLine` — empty body.
- `void ChooseOption(const FGuid& NodeGuid, FName Choice)` — `ChosenOptions.AddUnique(NodeGuid, Choice)` guarded by `WasOptionChosen`.
- `bool WasOptionChosen(const FGuid& NodeGuid, FName Choice)` — `ChosenOptions.FindPair(NodeGuid, Choice) != nullptr`.
- `void OnShowChoiceDialogueLine(const FGuid& ChoiceSectionID, const FZWDialogueData& DialogueData)` — stores `LastChoiceDialogueLine.Key/Value`, clears `LastDialogueLine = {}`.
- `const FZWDialogueData& GetDialogueLineToShowDuringChoice(const FGuid& ChoiceSectionID) const` — returns the last choice line if the section matches (`LastChoiceDialogueLine` check and value-unwrap are dead code — the branch doesn't stash its result) otherwise `LastDialogueLine`.
- `void SetPanelWidgetData(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& ChoiceData)` — broadcasts `ChoiceStarted`.
- `void ReceiveDataFromQuestChoicePanelWidget(UZWDialogueChoiceChangeableObject* ChoiceData)` — casts to `UZWDialogueChoicePanelWidgetData` into a local (`Cast` result unused; stub/unwired).
- Private: `ResetChosenOptions()`, `ResetLastDialogueLines()` (defined but **not called anywhere in reviewed .cpp** — reset hooks not wired).
- State: `TMultiMap<FGuid, FName> ChosenOptions`, `FZWDialogueData LastDialogueLine`, `TPair<FGuid, FZWDialogueData> LastChoiceDialogueLine`.

### 4.4 Module `ZWDialogueSystem` — UI (`Source/ZWDialogueSystem/Public/UI/`)

#### `EChoiceSelection` → `UENUM()` (`ZWDialogueChoicePanelWidget.h`)
`Current`, `Next`, `Previous`.

#### `UZWDialogueChoiceWidget` → `UCLASS()` base `UCommonActivatableWidget` (`ZWDialogueChoiceWidget.h/.cpp`)
Single choice row.
- `UPROPERTY(EditAnywhere, meta=(BindWidget)) UTextBlock* ChoiceBoxText = nullptr`
- `void SelectChoice()` → sets text color `FLinearColor::Gray` (selected)
- `void UnselectChoice()` → sets text color `FLinearColor::White`
- `void SetChoiceData(UZWDialogueChoiceData* ChoiceData)` → `ChoiceBoxText->SetText(ChoiceData->ChoiceText)`
- `FName ChooseAndGetChoiceLabel()` → returns display text as FName (`FName(ChoiceBoxText->GetText().ToString())`; the CorrectLabel assignment is commented out in the panel, so the confirm path uses positional names instead).

#### `UZWDialogueChoicePanelWidget` → `UCLASS()` base `UCommonActivatableWidget` (`ZWDialogueChoicePanelWidget.h/.cpp`)
Navigatable vertical choice list.
- Public:
  - `void SetupView(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& ChoiceData)` — casts+checks data, resets `CurrentChoiceIndex=0`, spawns a choice widget for each `MainChoices` item, then each `Choices` item, then `SelectChoice(Current)`.
  - `UFUNCTION(BlueprintCallable, Category = DialogueUI) void SelectAndConfirmChoiceAtIndex(int Index)`
  - `const TMap<int, UZWDialogueChoiceWidget*>& GetChoices() const`
  - `void ShowDialogueChoicePanelWidget()` / `void HideDialogueChoicePanelWidget()` — Show only sets `bIsActive` (the `ActivateWidget()`/`SetFocus()` calls are commented out); Hide calls `DeactivateWidget()` and clears state.
  - `UZWDialogueChoicePanelWidgetData* GetChoiceWidgetData()`
- Protected/virtual:
  - `NativeOnActivated()` — registers CommonUI UI action bindings (`RegisterUIActionBinding(FBindUIActionArgs(...))`) for `NextChoiceActionData`, `PreviousChoiceActionData`, `ConfirmChoiceActionData` (all `FDataTableRowHandle`, `EditDefaultsOnly, Category="Input Actions Data"`) if handles invalid and data not null; stores `FUIActionBindingHandle`s.
  - `NativeOnDeactivated()` — resets `bIsActive`, empties `Choices`, clears `ChoiceWidgetData`, `ChoicesBox->ClearChildren()`, resets `bAlreadySelectedChoice`.
  - `UFUNCTION(BlueprintCallable) void SelectNextChoice()` / `SelectPreviousChoice()` / `ConfirmSelectedChoice()`.
  - `ConfirmSelectedChoice()` — one-shot guard `bAlreadySelectedChoice`; writes `ConfirmedChoice = FName("Choice " + FString::FromInt(CurrentChoiceIndex))` *— the label from the widget text (`ChooseAndGetChoiceLabel`) is bypassed with a commented-out assignment*; then `ChoiceWidgetData->SetDirty(true)` to broadcast `OnChange`.
  - `UPROPERTY(EditAnywhere, meta=(BindWidget)) UVerticalBox* ChoicesBox = nullptr`
  - `UPROPERTY(EditAnywhere) TSubclassOf<UZWDialogueChoiceWidget> ChoiceWidgetRef`
  - `int32 CurrentChoiceIndex = 0`, `bool bIsActive = false`
- Private:
  - `void CreateChoice(UZWDialogueChoiceData* ChoiceData, int32 Index)` — skip when `bSingleUse && bWasChosen`; `CreateWidget(this, ChoiceWidgetRef)`; `AddChildToVerticalBox`; `SetChoiceData`; insert into `Choices` map (key = index).
  - `void SelectChoice(EChoiceSelection ChoiceToSelect)` — highlight based on unit move within `Choices.Num()` bounds (no-op at boundary indices).
  - `UZWDialogueChoicePanelWidgetData* ChoiceWidgetData` (UPROPERTY), `TMap<int, UZWDialogueChoiceWidget*> Choices`, `bool bAlreadySelectedChoice = false`.

#### `FZWDialogueWidgetData` → `USTRUCT(BlueprintType)` (`ZWMovieSceneDialogueWidget.h`)
- `FText Speaker` (Category "SpeakerID"), `FText DialogueLine` (Category "Dialogue Line").

#### `UZWMovieSceneDialogueWidget` → `UCLASS(Abstract)` base `UUserWidget`, implements `IZWDialogueLineHandler` (`ZWMovieSceneDialogueWidget.h/.cpp`)
THE subtitle/dialogue display widget. It auto-registers itself as a dialogue handler:
- `bool Initialize()` — `GameInstance->GetSubsystem<UZWMovieSceneDialogueSubsystem>()->RegisterDialogueHandler(this)`.
- `void SetDialogueData(const FZWDialogueWidgetData& DialogueData)` — sets `Speaker` text and `DialogueText` text; when speaker empty, `Speaker`/`Dots` collapsed else visible (a typewriter "..." indicator).
- `FGuid GetDialogueEventID() const`, `uint32 GetOrder() const` → **`150u`** ("should be the last one" — biggest handler order so UI is the last to react).
- `EZWStartDialogueResult OnStartDialogueLine(...)` — caches `DialogueEventID`; speaker falls back from `DialogueData.SpeakerID` when `Speaker` empty (`FText::FromName`); `SetDialogueData(...)`; `DialogueText->SetVisibility(Visible)`; returns `Handled`.
- `void OnFinishDialogueLine(...)` — resets `DialogueEventID = FGuid()`, blanks text, `DialogueText->SetVisibility(Hidden)`.
- `void NativeDestruct()` — unregisters from subsystem.
- Bounded widgets: `TObjectPtr<UTextBlock> Speaker`, `DialogueText`, `Dots` (all `EditAnywhere, meta=(BindWidget)`).
- `OnDialogueLineUpdated` is intentionally not overridden (commented out — animation support future work).

### 4.5 Module `ZWDialogueSystem` — Settings (`Config`)

#### `UZWDialogueSettings` → `UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Dialogue System"))` base `UDeveloperSettings` (`ZWDialogueSettings.h/.cpp`), ctor empty
Google Cloud TTS tooling config (Project Settings → "ZW Dialogue System"):
- `FString TTSGeneratorApiKey` (`Config, EditAnywhere`, Category TTS) — Google Cloud TTS API key.
- `FDirectoryPath BaseAudioExportPath` (`Config, EditAnywhere`, Category Paths) — main folder for generated WAVs (comment mentions e.g. "Localization/Audio"); note the generator (`FZWDialogueAudioGenerator`) actually hardcodes `Content/Localization/Audio`, ignoring this setting in the current code.
- `FString DefaultLanguageCode` (Category TTS) — e.g. "db-db" or "pl-PL".
- `FString DefaultVoiceName` (Category TTS) — e.g. "pl-PL-Wavenet-B".

### 4.6 Module `ZWDialogueSystemEditor` (`Source/ZWDialogueSystemEditor/Public/`)

#### `FZWDialogueSystemEditorModule` → `IModuleInterface` (`ZWDialogueSystemEditor.h/.cpp`)
Empty module shell, `IMPLEMENT_MODULE(FZWDialogueSystemEditorModule, ZWDialogueSystemEditor)`.

#### `FZWDialogueAudioGenerator` → plain shared class, `TSharedFromThis<FZWDialogueAudioGenerator>` (`ZWDialogueAudioGenerator.h` + `ZWDialogueAudioGenerator.cpp`)
**Editor-side Google TTS client.** `ZWDIALOGUESYSTEMEDITOR_API`, not UCLASS.
- Delegate: `DECLARE_DELEGATE_TwoParams(FOnTTSRequestCompleted, const FZWDialogueData& /*UpdatedData*/, bool /*bSuccess*/)` (non-dynamic shared delegate).
- `static TSharedRef<FZWDialogueAudioGenerator> Create()` → `MakeShared<FZWDialogueAudioGenerator>()`.
- **`void Execute(const FZWDialogueData& InData, const FString& ApiKey, const FString& LangCode, FOnTTSRequestCompleted InCallback)`** — full GenerateAudio scenario:
  1. Guards/copy: caches `WorkingData = InData`, `TargetLang = LangCode`, `CompletionCallback`. If `DialogueLine` empty → immediate `CompletionCallback.ExecuteIfBound(WorkingData, false)`. If `AudioData.AudioGuid` invalid → `WorkingData.AudioData.AudioGuid = FGuid::NewGuid()` (each generated line gets a stable GUID = its filename).
  2. Builds the Google TTS JSON body with `FJsonObject`:
     - `input.text = DialogueLine.ToString()`
     - `voice.languageCode = LangCode`; `voice.name` = `"pl-PL-Wavenet-B"` when `LangCode == "pl-PL"` else `"en-GB-Chirp3-HD-Aoede"` (hardcoded picker, comment says professional tooling would select voice per SpeakerId),
     - `audioConfig.audioEncoding = "LINEAR16"` (yields plain WAV), `audioConfig.sampleRateHertz = 48000`.
     - Serialized with `TJsonWriter` into `JsonPayload`.
  3. HTTP request: `FHttpModule::Get().CreateRequest()`; URL `https://texttospeech.googleapis.com/v1/text:synthesize?key=<ApiKey>`; verb POST; `Content-Type: application/json`; content = JsonPayload. Target save path precomputed as `Content/Localization/Audio/<LangCode>/<Guid>.wav`.
  4. Lifetime management: `TSharedRef<FZWDialogueAudioGenerator> StrongThis = AsShared()` captured in a lambda bound to `OnProcessRequestComplete()` so the generator survives until the response (helps against dangling after the caller returns `Execute`).
  5. `Request->ProcessRequest()`.
- **`void OnTTSResponseReceived(FHttpRequestPtr, FHttpResponsePtr, bool bWasSuccessful)`**:
  - On HTTP 200: parse JSON, read base64 `audioContent`, `FBase64::Decode` into `TArray<uint8> AudioBytes`.
  - If `AudioBytes.Num() > 44`: reads `uint32 ByteRate` via `FMemory::Memcpy(&ByteRate, AudioBytes.GetData() + 28, 4)` (WAV fmt-chunk `dwAvgBytesPerSec` field), computes `PrecalculatedDuration = (AudioBytes.Num() - 44) / ByteRate` (skips the 44-byte canonical header; if `ByteRate == 0` duration stays 0).
  - Saves via `FFileHelper::SaveArrayToFile(AudioBytes, *SavePath)` into `Content/Localization/Audio/<TargetLang>/<Guid>.wav`; success flips `bSuccess = true`.
  - Failure path: logs `"Błąd TTS: %s"` with response body (or "Brak odpowiedzi").
  - Always fires `CompletionCallback.ExecuteIfBound(WorkingData, bSuccess)` so the caller persists the updated `FZWDialogueData` (new GUID + duration).

### 4.7 Nested sub-plugin — module `ZWMovieSceneDialogueTrack` (both copies: `Source/ZWMovieSceneDialogueTrack/` outer copy + `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/` sub-plugin copy)

The two source trees are **near-identical (parallel copies)** of the Sequencer dialogue-track classes. Differences are listed explicitly after the shared description. `Source/ZWMovieSceneDialogueTrack/Public` contains only `ZWMovieSceneDialogueSection.h` + a template private cpp — i.e. the outer-level copy is partial (only the runtime section/mailbox pieces exist there), while the sub-plugin copy is complete.

#### Shared class set (present in the sub-plugin copy `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/Public/.h`, and partially in `Source/ZWMovieSceneDialogueTrack/Public/`):

##### `FZWDialogueData` → `USTRUCT()` (`ZWMovieSceneDialogueSubsystem.h`, sub-plugin variant)
Minimal local redefinition (does not import the parent's type):
- `FGuid EventID`; `FText Speaker` (`EditAnywhere`); `FText DialogueLine` (`EditAnywhere`); `TWeakInterfacePtr<IZWDialogueLineHandler> FinalDialogueLineHandler = nullptr`.
- No `SpeakerID`, no `AudioData` — variants differ from the parent module's line data.

##### `EZWStartDialogueResult` → plain enum — duplicated in the sub-plugin header (`Handled / Unhandled / Final`).

##### `UZWDialogueLineHandler` / `IZWDialogueLineHandler` → `UINTERFACE(Blueprintable)` — duplicate of the interface (same methods) but with the `ZWMOVIESCENEDIALOGUETRACK_API` export macro.

##### `UZWMovieSceneDialogueSubsystem` → `UCLASS()` base `UGameInstanceSubsystem` (`ZWMovieSceneDialogueSubsystem.h/.cpp`, sub-plugin variant)
Simpler than the parent variant (no audio import branch):
- `FDialogueTickEvent DialogueTickEvent` — `DECLARE_MULTICAST_DELEGATE_TwoParams(const FGuid, float)` (Started/Ended declared in comments only).
- `virtual void Deinitialize()` — empties handlers.
- `void TickDialogue(FGuid EventID, float CurrentTime)` — broadcasts `DialogueTickEvent` only.
- `FZWDialogueTokenPtr TriggerDialogueLine(const FZWDialogueDetails& DialogueDetails) Overload` — NewGuid EventID; copies `Speaker`/`DialogueLine` (no SpeakerID); forwards.
- `FZWDialogueTokenPtr TriggerDialogueLine(FZWDialogueData)` — synchronous: runs all `OnStartDialogueLine` handlers (terminate at first `Final`), appends to `CurrentDialogueLines`, returns token. No audio, no `PendingAudioDialogues` paths.
- `void CloseDialogueLine(const FGuid&)` — notifies `OnFinishDialogueLine` up to `FinalDialogueLineHandler`, removes the entry (`check` when line missing — hazard when double-closing).
- `RegisterDialogueHandler` / `UnregisterDialogueHandler` — same sorting/`ensureMsgf` by `GetOrder()` semantics as the parent variant.
- `operator==(FZWDialogueData, FGuid)` free function.
- `friend FZWDialogueToken` (dtor → `CloseDialogueLine`).
- `TArray<FZWDialogueData> CurrentDialogueLines`, `TArray<TWeakInterfacePtr<IZWDialogueLineHandler>> DialogueLineHandlers`.

##### `FZWDialogueToken` — same contract as the parent variant (EventID + weak subsystem; dtor closes dialogue line).

##### `FZWDialogueDetails` → `struct` — minimal variant: `FText Speaker`, `FText DialogueText`, `float Progress /* In Seconds */` (no `SpeakerID`, unlike the parent-side Details).

##### `UZWMovieSceneDialogueTrack` → `UCLASS()` base `UMovieSceneNameableTrack, IMovieSceneTrackTemplateProducer` (`ZWMovieSceneDialogueTrack.h/.cpp`)
The Sequencer track type:
- `virtual FMovieSceneEvalTemplatePtr CreateTemplateForSection(const UMovieSceneSection&) const override` — when section `IsA<UZWMovieSceneDialogueSection>` returns `FZWMovieSceneDialogueSectionTemplate(*CastChecked<...>(&InSection))`.
- Generic `UMovieSceneTrack` overrides: `AddSection`, `SupportsType` (child-of `UZWMovieSceneDialogueSection`), `CreateNewSection()` → `NewObject<UZWMovieSceneDialogueSection>(this, NAME_None, RF_Transactional)`, `GetAllSections`, `HasSection`, `IsEmpty`, `RemoveAllAnimationData`, `RemoveSection`, `RemoveSectionAt`.
- `virtual bool SupportsMultipleRows() const override { return true; }` — several dialogue rows in one track.
- `#if WITH_EDITORONLY_DATA virtual FText GetDisplayName() const override` → LOCTEXT `"TrackName", "Dialogue"`.
- Storage: `UPROPERTY() TArray<UMovieSceneSection*> Sections`.

##### `UZWMovieSceneDialogueSection` → `UCLASS()` base `UMovieSceneSection`
- Sub-plugin variant (`ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/Public/ZWMovieSceneDialogueSection.h`): `FText Speaker = FText::FromString("")` (EditAnywhere, Category Speaker), `FText DialogueText = FText::FromString("")` (EditAnywhere, Category Dialogue).
- Outer-copy variant (`Source/ZWMovieSceneDialogueTrack/Public/ZWMovieSceneDialogueSection.h`): replaces raw fields with `UPROPERTY(EditAnywhere) FZWDialogueData DialogueData;` (parent plugin's struct, thus audio-enabled); old per-field properties (`EventID`, `Speaker`, `LocalizedSpeaker`, `DialogueText`) kept commented out.

##### `FZWMovieSceneDialogueSectionTemplate` → `USTRUCT()` base `FMovieSceneEvalTemplate` (`ZWMovieSceneDialogueSectionTemplate.h/.cpp`)
The template bridging Sequencer evaluation to the dialogue subsystem:
- `FZWMovieSceneDialogueSectionTemplate(const UZWMovieSceneDialogueSection& InSection)` stores `Section`.
- `GetScriptStructImpl()` returns `*StaticStruct()`.
- `virtual void Initialize(const FMovieSceneEvaluationOperand&, FMovieSceneContext&, FPersistentEvaluationData&, IMovieScenePlayer&) const override` — records `Operand.SequenceID` into `FZWMovieSceneDialogueEditorTemplatePersistentData` (a `IPersistentEvaluationData` wrapper storing `FMovieSceneSequenceID`).
- `virtual void Evaluate(...) const override` — caches `check(Section)`, builds a data snapshot for the frame, and adds the `FZWMovieSceneDialogueExecutionToken`:
  - Sub-plugin variant: builds `FZWDialogueDetails DialogueData{ .Speaker = Section->Speaker, .DialogueText = Section->DialogueText }` (NodeGuid/FlowNode linking left as commented TODO about `UFlowNode`).
  - Outer variant: passes `Section->DialogueData` (full `FZWDialogueData`).
  - `SetupOverrides()`: `EnableOverrides(RequiresTearDownFlag)`.
  - `TearDown(FPersistentEvaluationData&, IMovieScenePlayer&) const override`: empty body.
- Execution token (file-local struct in the .cpp, `final`):
  - Holds `TObjectPtr<const UZWMovieSceneDialogueSection> DialogueSection` + snapshot (`FZWDialogueDetails` or `FZWDialogueData`).
  - `virtual void Execute(const FMovieSceneContext&, const FMovieSceneEvaluationOperand&, FPersistentEvaluationData&, IMovieScenePlayer&) override`:
    1. `PersistentData.GetOrAddSectionData<FZWMovieSceneDialoguePersistentData>()` (wrapper storing `FZWDialogueTokenPtr Token`).
    2. Resolves `UWorld` from `Player.GetPlaybackContext()`, obtains `UZWMovieSceneDialogueSubsystem` from the GameInstance (subsystem class resolves to the module-local one — parent copy resolves parent's, sub-plugin copy resolves its own).
    3. If no token yet → `DialogueSubsystem->TriggerDialogueLine(DialogueData)`.
    4. Then `DialogueSubsystem->TickDialogue(Token->EventID, ...)` — feeding either `DialogueData.Progress` (sub-plugin variant) or `DialogueData.AudioData.PrecalculatedDuration` (outer variant).

##### `UZWMovieSceneDialogueWidget` → `UCLASS(Abstract)` base `UUserWidget, IZWDialogueLineHandler` (`ZWMovieSceneDialogueWidget.h/.cpp`, sub-plugin variant)
Sub-plugin's subtitle widget — same surface as the parent-module widget: `Initialize` registers with the (sub-plugin-local) subsystem, `SetDialogueData(const FZWDialogueWidgetData&)` handles Speaker/Dots visibility, `GetDialogueEventID`, `GetOrder()` = 150u, `OnStartDialogueLine` returns `Handled`, `OnFinishDialogueLine` blank-out, `NativeDestruct` unregister. `FZWDialogueWidgetData` struct duplicated in the same header (Categories "Speaker"/"Dialogue Line" instead of "SpeakerID").

##### `UZWMovieSceneDialogueTrackInstance` → `UCLASS()` base `UMovieSceneTrackInstance` (`ZWMovieSceneDialogueTrackInstance.h/.cpp`)
- `virtual void OnInputAdded(const FMovieSceneTrackInstanceInput& InInput) override`, `OnInputRemoved(...)`, `OnDestroyed()` — **all essentially empty/stubbed** (the interesting body — Widget retrieval via `GetLinker()` + AssetRegistry — is commented out); member `UZWMovieSceneDialogueWidget* MovieSceneDialogueWidget = nullptr` (unused currently).

##### `FZWMovieSceneDialogueTrackModule` → `IModuleInterface` (`ZWMovieSceneDialogueTrackModule.h/.cpp`, sub-plugin module cpp)
Empty module shell, `IMPLEMENT_MODULE(FZWMovieSceneDialogueTrackModule, ZWMovieSceneDialogueTrack)`.

#### Duplicated header-only classes in the sub-plugin (Quest choice) — `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/Public/`

An entire **second copy of the choice system** with a *Quest* prefix (see the parent's 4.x versions above; the sub-plugin's variants are API-identical in structure, renamed with the `Quest` prefix, minus any call into `ZWDialogueSystem`):

- `FDialogueChoice` / `FChoiceData` → `USTRUCT()` ×2 (`ChoiceData.h/.cpp`) — same layout as `FZWDialogueChoice`/`FZWChoiceData`; `TitleProperty = "SocketName"`.
- `UQuestChoiceChangeableObject` → `UCLASS()` base `UObject` — same SetDirty/ClearDirty/BroadcastValueChanged contract.
- `UQuestChoiceData` → `UCLASS()` base `UQuestChoiceChangeableObject` — same choice fields.
- `UQuestChoicePanelWidgetData` → `UCLASS()` base `UQuestChoiceChangeableObject` — `MainChoices`/`Choices`/`ConfirmedChoice`.
- `UQuestChoiceWidget` → `UCLASS()` base `UCommonActivatableWidget` — same Select/Unselect (Gray/White) and `ChooseAndGetChoiceLabel` logic.
- `UQuestChoicePanelWidget` → `UCLASS()` base `UCommonActivatableWidget`, `EChoiceSelection` UENUM duplicated here too — same SetupView / SelectAndConfirmChoiceAtIndex / input-binding lifecycle; names differ: `ShowQuestChoicePanelWidget`/`HideQuestChoicePanelWidget`, `ReceiveDataFromQuestChoicePanelWidget`, etc.
- `UQuestChoiceSubsystem` → `UCLASS()` base `UGameInstanceSubsystem`, implements the **sub-plugin's** `IZWDialogueLineHandler` — same states and methods (`GetOrder()=11u`, `ChooseOption`, `WasOptionChosen`, `OnShowChoiceDialogueLine`, `GetDialogueLineToShowDuringChoice` with the same dead `if` branch, `SetPanelWidgetData`, `ReceiveDataFromQuestChoicePanelWidget` cast stub, `ResetChosenOptions`, `ResetLastDialogueLines` unwired).
- `UQuestChoiceSubsystem` depends only on its own subsystem — references `UZWMovieSceneDialogueSubsystem` from the sub-plugin module (natural, since this subsystem is compiled inside the same plugin where the runtime module lives).

> **Conclusion:** the sub-plugin has its **own parallel duplicate of the entire dialogue-subsystem + choice-system stack**, indistinguishable from the parent's except for a `Quest`-prefixed naming of the choice part and the exclusion of audio. The outer-level copy located at `Source/ZWMovieSceneDialogueTrack/` only carries a single public header + template cpp and relies on the sub-plugin's sources for the rest of these classes (a source-unification risk — see §8).

### 4.8 Nested sub-plugin — module `ZWMovieSceneDialogueTrackEditor` (`ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrackEditor/`)

(The outer plugin has an equivalent file `Source/ZWMovieSceneDialogueTrackEditor/Private/ZWMovieSceneDialogueTrackEditor.cpp` alone — no public header — with the same content.)

#### `FZWMovieSceneDialogueTrackEditorModule` → `IModuleInterface` (`ZWMovieSceneDialogueTrackEditorModule.h/.cpp`)
- `FDelegateHandle DialogueTrackCreateEditorHandle`.
- `StartupModule()` — loads `"Sequencer"` module, `SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FZWDialogueTrackEditor::CreateTrackEditor))` and stores the handle (comment mentions "register Flow sequence track").
- `ShutdownModule()` — `SequencerModule.UnRegisterTrackEditor(DialogueTrackCreateEditorHandle)`.
- `IMPLEMENT_MODULE(FZWMovieSceneDialogueTrackEditorModule, ZWMovieSceneDialogueTrackEditor)`.

#### `FZWDialogueTrackEditor` → `class FMovieSceneTrackEditor` (`ZWMovieSceneDialogueTrackEditor.h/.cpp`), `ZWMOVIESCENEDIALOGUETRACKEDITOR_API`
Header banner attributes copyright to https://github.com/MothCocoon/DialogueGraph/contributors (the track editor is derived from DialogueGraph's Sequencer track editor).
- `static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer)` — factory bound from the module's StartupModule.
- `FZWDialogueTrackEditor(TSharedRef<ISequencer> InSequencer)` — forwards to `FMovieSceneTrackEditor`.
- `virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override` — adds a `"Dialogue Track"` menu entry (`AddTooltip`: "Adds a new Dialogue track.") with icon `FAppStyle ... "Sequencer.Tracks.Audio"` **only when** `FMovieSceneSequenceEditor::Find(...)->SupportsEvents(RootMovieSceneSequence)` (i.e. only in event-capable sequences).
- `virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override` → `Type == UZWMovieSceneDialogueTrack::StaticClass()`.
- `virtual bool SupportsSequence(UMovieSceneSequence* InSequence) const override` → only sequences `IsChildOf(ULevelSequence::StaticClass())`.
- `virtual const FSlateBrush* GetIconBrush() const override` → `FAppStyle::GetBrush("Sequencer.Tracks.Event")`.
- `virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override` — an `FSequencerUtilities::MakeAddButton(...,"Section", ...)` that calls `CreateNewSection(Track, RowIndex+1, UZWMovieSceneDialogueSection::StaticClass(), bSelect=true)`.
- `virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override` → `new FZWDialogueSection(SectionObject, GetSequencer())`.
- Private `AddDialogueSubMenu(FMenuBuilder&)` → empty (unused, sub-menu stub).
- `void HandleAddDialogueTrackMenuEntryExecute()` — null/read-only guards, `FScopedTransaction("Add Dialogue Track")`, `FocusedMovieScene->AddTrack<UZWMovieSceneDialogueTrack>()` + `ensure`, `SetDisplayName(LOCTEXT "DialogueTrackName", "Dialogue")`, `GetSequencer()->OnAddTrack(NewTrack, FGuid())`.
- `void CreateNewSection(UMovieSceneTrack* Track, int32 RowIndex, UClass* SectionType, bool bSelect) const` — transaction "Add Dialogue Section"; computes `PlaybackEnd = UE::MovieScene::DiscreteExclusiveUpper(GetPlaybackRange())`; bumps the row index of all existing sections at/below insertion row; picks `OverlapPriority = max(other's OverlapPriority + 1, 0)`; `NewSection->SetRange(TRange<FFrameNumber>(CurrentTime.Time.FrameNumber, PlaybackEnd))` — **new section spans from the current playhead to the sequence end** (drag to shorten afterwards); `AddSection`, `UpdateEasing`, optional selection/throb, `NotifyMovieSceneDataChanged(MovieSceneStructureItemAdded)`; carries a TODO: "Dialogues can be on one track if they don't overlap".

#### `FZWDialogueSection` → `ISequencerSection` (same header)
Row painter for a Dialogue section in the Sequencer track:
- Ctor stores `UMovieSceneSection& Section` (`TWeakPtr<ISequencer>` unused in body).
- `virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override` — background tinted `FColor(134, 103, 106, 150)` (plum-ish) with `PaintSectionBackground`; code comment `@TODO: ColorTint do zmiany` (color to change).
- `virtual UMovieSceneSection* GetSectionObject() override` → `&Section`.
- `virtual FText GetSectionTitle() const override` — renders `"Speaker: DialogueText"` from the underlying section (`DialogueData.Speaker`/`DialogueData.DialogueLine` in the outer flattened variant, raw `Speaker`/`DialogueText` fields in the sub-plugin variant); blank when section not castable.
- `virtual float GetSectionHeight() const override` → `40.0f`.

---

## 5. Implementation (Private) notes

- **Module entry points:** `FZWDialogueSystemModule`, `FZWDialogueSystemEditorModule`, `FZWMovieSceneDialogueTrackModule`, `FZWMovieSceneDialogueTrackEditorModule` — all with empty startup/shutdown bodies (only the *editor track* module registers an editor hook at load: the Sequencer `RegisterTrackEditor` / `UnRegisterTrackEditor` pair).
- **`ZWMovieSceneDialogueSubsystem.cpp` (outer)** is the runtime brain — see §4.2 for full trigger/tick/close/import semantics. Key design: `FZWDialogueToken` RAII-style closure; async WAV import decoupled from Sequencer playback; `PendingAudioDialogues` acts as a mailbox shielding UI until decoded audio actually starts (or fails).
- **`ZWMovieSceneDialogueSectionTemplate.cpp`** implements the full MovieScene eval-template pattern (`PersistentData` wrappers + execution token) in both source copies.
- **Choice panel cpp** is a CommonUI-idiomatic activatable widget (action binding registration from datatables via `FDataTableRowHandle`).
- **Stub/wired-off places discovered while reading privates:** `AddDialogueSubMenu` empty, `ReceiveDataFromQuestChoicePanelWidget` cast-unused, `ResetChosenOptions`/`ResetLastDialogueLines` never invoked, `GetDialogueLineToShowDuringChoice` comparison branch yields no value, `ChooseAndGetChoiceLabel` unused at confirm (hard-coded `"Choice <index>"` names), `UZWMovieSceneDialogueTrackInstance` callbacks empty, several commented-out `NativeOnMouseWheel`/`NativeOnKeyDown`/`GetDesiredInputConfig` overrides.

## 6. Configuration (.ini)

- `Config/DefaultZWMovieSceneDialogueTrack.ini` — contains only `[CoreRedirects]` with no entries. **"brak"** beyond that.
- Static, engine-defined config lives in `UZWDialogueSettings` (`Config=Game`, defaultconfig — so user content goes to `DefaultGame.ini`):
  - `TTSGeneratorApiKey` (FString)
  - `BaseAudioExportPath` (FDirectoryPath)
  - `DefaultLanguageCode` (FString)
  - `DefaultVoiceName` (FString)
  - Note: although `BaseAudioExportPath`, `DefaultLanguageCode` and `DefaultVoiceName` are exposed, the shipped `FZWDialogueAudioGenerator::Execute` and `UZWMovieSceneDialogueSubsystem::TriggerDialogueLine` use their own compiled defaults (`LangCode` parameter, `"en-gb"`, `pl-PL` special-case voice, path `Localization/Audio`) — the settings above are ready but currently not consumed by the plugin's internal code paths (intended to be used by future editor tooling such as a settings panel / blueprint editors tying into `BlueprintEditorLibrary`).

## 7. Internal ZWSuite dependencies

All dependency references resolved by scanning every `Build.cs` and every `#include` in the source tree:

- `ZWDialogueSystem` runtime module ← `RuntimeAudioImporter` plugin (`URuntimeAudioImporterLibrary`, `UImportedSoundWave`, `ERuntimeImportStatus`, `ERuntimeAudioFormat` in `UZWMovieSceneDialogueSubsystem.h/.cpp`), CommonUI (all choice widgets), GameplayTags-dependency: declared in Build.cs, though no direct GameplayTags headers are included in sources (reserved).
- `ZWDialogueSystemEditor` depends on `ZWDialogueSystem` runtime module (public) — the TTS generator consumes `FZWDialogueData` from it.
- The **outer-level** `Source/ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.Build.cs` depends on `ZWDialogueSystem`; this is the *outer* copy — the one matching the outer plugin's flattened layout (its section stores `FZWDialogueData`).
- The **sub-plugin copy** (`ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack/ZWMovieSceneDialogueTrack.Build.cs`) has **no** reference to `ZWDialogueSystem` — self-contained.
- No cross-references to other ZWSuite plugins (no `#include` or Build.cs references to e.g. other ZW* modules) — only internal references into the same plugin tree. ("brak" for anything else.)
- External non-Frostbyte plugin references: `RuntimeAudioImporter` (marketplace plugin), `CommonUI` (engine), `AssetSearch` (engine editor plugin; depended on by nested editor only and via outer-module dep), `EditorScriptingUtilities` (engine plugin), `BlueprintEditorLibrary` (engine). Engines' Sequencer/MovieScene/LevelSequence infra is used throughout.

---

## 8. Notes / risks

- **Source duplication hazard:** the dialogue-track classes exist in **two source trees** (`Source/ZWMovieSceneDialogueTrack` at the outer plugin level and `ZWMovieSceneDialogueTrack/Source/ZWMovieSceneDialogueTrack` inside the sub-plugin) and the public headers do not simply reuse the sub-plugin's classes. Notably `Source/ZWMovieSceneDialogueTrack/Public/ZWMovieSceneDialogueSubsystem.h` does **not exist** at the outer level — the partial outer layout is maintained by depending the outer Build.cs on `ZWDialogueSystem` and shipping class *copies*. Meanwhile the sub-plugin ships **prebuilt Win64 DLLs** whose provenance/parity with the current sources is unverifiable from source alone (risk of mismatch when rebuilding only one copy).
- **Symbol shadowing:** `FZWDialogueData`, `FZWDialogueDetails`, `FZWDialogueToken`, `IZWDialogueLineHandler`, `EZWStartDialogueResult`, `UZWMovieSceneDialogueSubsystem`, `UZWMovieSceneDialogueWidget`, `EChoiceSelection` exist in the parent module and again (local variants) in the sub-plugin module. Both are exported DLLs; when both plugins are loaded in the same editor there is no linker-level conflict (different symbols) but Blueprint-collateral/UStruct UE reflection collisions between same-named USTRUCTs (`FZWDialogueData`, `FZWDialogueWidgetData`) are possible in cook/blueprint contexts; it is a known duplicated-type risk.
- **Hard-coded paths/strings** in runtime/dialogue audio: `"Localization/Audio"` subfolder and `"en-gb"` language inside `UZWMovieSceneDialogueSubsystem::TriggerDialogueLine()`; `"pl-PL-Wavenet-B"` / `"en-GB-Chirp3-HD-Aoede"` in `FZWDialogueAudioGenerator`; assume no localization plug-in runtime so the `PrecalculatedDuration` is treated as the time reference for ticking.
- **Dead/unfinished paths**: dead `if` value in `GetDialogueLineToShowDuringChoice`, unwired reset functions, cast-unused receive function, empty `AddDialogueSubMenu`, empty template `TearDown`, never-broadcast `DialogueStartedEvent`/`DialogueEndedEvent` (commented-out TODOs).
- **Hard `check`s**: `TriggerDialogueLine` and `TickDialogue`/`CloseDialogueLine` `check(CloseDialogueData != nullptr)` — calling CloseDialogueLine twice with the same EventID, or outside `TickDialogue`, can crash; the token dtor (friend `FZWMovieSceneDialogueSubsystem`) must therefore drop in the right order.
- **`ConfirmedChoice` doesn't use widget text** — it uses a positional name `"Choice <index>"`, so choice persistence should not rely on user-visible text stability.
- `UZWDialogueChoiceSubsystem::ReceiveDataFromQuestChoicePanelWidget` does a Cast without consuming the output — a candidate orphan API ("quest" naming suggests it was copied from the quest twin inside the sub-plugin tree).
- There is also evidence of the plugin being extracted from / influenced by **Flow**-style design (commented-out `UFlowNode` usage in `ZWMovieSceneDialogueSectionTemplate` and `FZWDialogueChoice` QoL members referencing `GetConditionsString(const UFlowNode*)`) — a port that retains dialog-supply hooks from a Flow/DialogueGraph era.
- All User-facing text within the source (UPROPERTY comments, log messages) is mixed Polish/English — code comments partly Polish (e.g. "Klucz API do Google Cloud TTS", "Błąd dekodowania w wątku!").

---

*Documentation generated by reading every source file listed in the review root; "brak" means the item does not exist in the reviewed sources.*

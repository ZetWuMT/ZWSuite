# ZWQuestFactBase — Dokumentacja techniczna (Technical Documentation)

> Source of truth: `/opt/data/projectx/ZWSuite-src/ZWQuestFactBase` (all 24 source files read: `.uplugin`, 2 × `Build.cs`, 7 public headers, 11 private source files, 1 `.ini`, 2 resource files). All statements below are derived directly from that code; items absent from the code are marked "none" / "brak".

## 1. Przegląd (Overview)

**ZWQuestFactBase** ("Tool for managing the quest facts", v0.1) is a Unreal Engine plugin providing a quest-fact registry for the ZWSuite project. It consists of two responsibilities:

1. **Runtime registry** (`ZWQuestFactBase` module): `UZWQuestFact` / `UZWQuestFolder` — lightweight `UObject` assets representing quest facts and folder hierarchies, stored as assets under `/Game/QuestFacts`, plus `UZWQuestFactBaseSubsystem`, a `UGameInstanceSubsystem` that scans the Asset Registry at GameInstance init, loads every fact asset, and exposes Blueprint-callable read/write access to integer fact values with a value-changed multicast delegate.
2. **Editor tooling** (`ZWQuestFactBaseEditor` module): a dockable Nomad tab containing `SZWQuestFactBaseEditor` — a toolbar + search box + `STreeView` that lists fact assets as a tree (folders parented by `FGuid`), and can create / rename (F2 or double-click) / delete fact and folder assets. It also registers `FZWQuestFactNameCustomization`, a detail-panel property customization for `FZWQuestFactSearchableName` that replaces the plain FName editor with a searchable combo box populated via a `GetOptions` metadata function.

The hierarchy is **GUID-based, not object-reference-based**: a child fact stores its parent's `FactGuid` in `ParentId`; `SubFacts` arrays are populated at editor tree-build time (not persisted).

## 2. Metadane (.uplugin)

File: `ZWQuestFactBase.uplugin`

| Field | Value |
|---|---|
| FileVersion | 3 |
| Version | 1 |
| VersionName | `0.1` |
| FriendlyName | `ZWQuestFactBase` |
| Description | "Tool for managing the quest facts" |
| Category | `Other` |
| CreatedBy | `tiramisoo` |
| CreatedByURL / DocsURL / MarketplaceURL / SupportURL | empty |
| CanContainContent | `false` |
| Installed | `true` |

**Modules:**

| Name | Type | LoadingPhase |
|---|---|---|
| `ZWQuestFactBase` | Runtime | `PreDefault` |
| `ZWQuestFactBaseEditor` | Editor | `Default` |

**Plugin dependencies:** `EditorScriptingUtilities` (enabled).

## 3. Podmoduły (Build.cs)

### 3.1 `Source/ZWQuestFactBase/ZWQuestFactBase.Build.cs`

- `PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs`.
- **Public:** `Core`.
- **Private:** `Projects`, `InputCore`, `ToolMenus`, `CoreUObject`, `Engine`, `Slate`, `SlateCore`.
- **Editor-only (`Target.Type == TargetType.Editor`) public:** `EditorFramework`, `UnrealEd`, `EditorScriptingUtilities`, `PropertyEditor`. (Notable: the runtime module directly links `UnrealEd` when building editor targets.)

### 3.2 `Source/ZWQuestFactBaseEditor/ZWQuestFactBaseEditor.Build.cs`

- `PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs`.
- No public/private include paths configured (empty).
- **Public:** `Core`, `ZWQuestFactBase` (depends on the runtime module).
- **Private:** `Projects`, `InputCore`, `EditorFramework`, `UnrealEd`, `ToolMenus`, `CoreUObject`, `Engine`, `Slate`, `SlateCore`, `EditorScriptingUtilities`.
- `DynamicallyLoadedModuleNames`: empty.

## 4. Publiczny API — klasy

### Module 1: `ZWQuestFactBase` (Runtime)

#### `FZWQuestFactSearchableName` — USTRUCT (BlueprintType)
Declared in `ZWQuestFact.h`. A one-property wrapper making a `FName` usable with the editor's property-type customization mechanism.
- `UPROPERTY(EditAnywhere, BlueprintReadWrite) FName QuestFactName;`

#### `UZWQuestFact` — UCLASS (BlueprintType), base `UObject`
Export macro `ZWQUESTFACTBASE_API`. One fact "asset". Declared in `ZWQuestFact.h`.
- `const FPrimaryAssetType PrimaryAssetType;` — non-UPROPERTY const member (set only via constructor default; the ctor body in `.cpp` is empty, so in practice it equals an uninitialized/default `FPrimaryAssetType` — see §8).
- `FPrimaryAssetId GetPrimaryAssetId() const;` — returns `FPrimaryAssetId(PrimaryAssetType, FName(GetName()))`.
- `UZWQuestFact();` — empty body.
- `bool IsFolder();` — returns `bIsFolder`.
- `protected: bool bIsFolder = false;` — set true only in `UZWQuestFolder` ctor.
- `UPROPERTY(EditAnywhere) FGuid FactGuid;` — identity of the fact.
- `UPROPERTY(EditAnywhere) FName FactName;` — display/lookup name.
- `UPROPERTY(EditAnywhere) FGuid ParentId;` — GUID of the parent folder (hierarchy is by GUID).
- `FGuid GetGuid();` — returns `FactGuid`.
- `TArray<UZWQuestFact*> SubFacts;` — **not a UPROPERTY** (no GC/serialization tracking); populated at editor tree-rebuild time only.

#### `UZWQuestFolder` — UCLASS (BlueprintType), base `UZWQuestFact`
- `UZWQuestFolder();` — sets `bIsFolder = true`. No other additions.

#### `FZWQuestFactBaseModule` — `IModuleInterface`
Declared in `ZWQuestFactBase.h`; implemented in `ZWQuestFactBase.cpp`.
- `void StartupModule();`
- `void ShutdownModule();`
- `void PluginButtonClicked();` — invokes the nomad tab.
- `private: void RegisterMenus();` — registered via `UToolMenus::RegisterStartupCallback` but **has an empty body**.
- `TSharedRef<SDockTab> OnSpawnPluginTab(const FSpawnTabArgs&);`
- `TSharedPtr<FUICommandList> PluginCommands;` — declared but never mapped in this module.

Runtime behavior:
- `StartupModule`: initializes `FZWQuestFactBaseStyle`, calls `ReloadTextures()`, registers ToolMenus startup callback, registers a **nomad tab spawner** named `"ZWQuestFactBase"` (display name "QuestFactBase", `ETabSpawnerMenuType::Hidden`) whose content is just `SNew(SButton)`.
- `ShutdownModule`: unregisters ToolMenus callback/owner, shuts down style, unregisters nomad tab.
- `IMPLEMENT_MODULE(FZWQuestFactBaseModule, ZWQuestFactBase)`.

#### `FZWQuestFactBaseStyle` — Slate style set (runtime module, editor-style only asset use)
`ZWQuestFactBaseStyle.h/.cpp`.
- `static void Initialize();` `static void Shutdown();` `static void ReloadTextures();` `static const ISlateStyle& Get();` `static FName GetStyleSetName();` (returns `"QuestFactBaseStyle"`).
- `StyleInstance` — static `TSharedPtr<FSlateStyleSet>`; `Shutdown()` asserts `ensure(StyleInstance.IsUnique())`.
- Content root is **engine** `FPaths::EngineContentDir() / "Editor/Slate/"` (the plugin's own `Resources/` content-root line is commented out).
- Brushes registered:
  - `QuestFactBase.OpenPluginWindow` — SVG `Starship/Common/BrowseContent` @ 20 px
  - `QuestFactBase.Fact` — SVG `Starship/AssetIcons/Object_16` @ 16 px
  - `QuestFactBase.FolderClosed` — `Icons/FolderClosed` @ 16 px
  - `QuestFactBase.FolderOpen` — `Icons/FolderOpen` @ 16 px (registered but not used anywhere in code — only `FolderClosed` is queried).

#### `FZWQuestFactData` — USTRUCT (BlueprintType)
Declared in `ZWQuestFactBaseSubsystem.h`. Runtime cell for one fact.
- `UPROPERTY(BlueprintReadWrite) FName Name;`
- `UPROPERTY(BlueprintReadWrite) FGuid Guid;`
- `TSharedPtr<UZWQuestFact> Parent;` — never filled by current code (always null).
- `TArray<TSharedPtr<UZWQuestFact>> Children;` — never filled (always empty).
- `UPROPERTY(BlueprintReadWrite) int32 Value = DefaultFactValue;` (`const int32 DefaultFactValue = 0` is a file-scope constant).

#### `UZWQuestFactBaseSubsystem` — UCLASS, base `UGameInstanceSubsystem`
Export macro `ZWQUESTFACTBASE_API`. The gameplay-facing registry.
- `virtual void Initialize(FSubsystemCollectionBase& Collection) override;` — calls `RegisterFacts()`.
- `virtual void Deinitialize() override;` — empty.
- `FOnFactValueChanged OnFactValueChanged;` — `DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactValueChanged, FName, inName, int32, inValue)`; broadcast from `SetFactValue`.
- `TArray<FZWQuestFactData*> QuestFacts;` — raw `new`-allocated registry entries (see §8 for lifetime).
- `void RegisterFacts();` — not a UFUNCTION. Loads `AssetRegistry` module, `GetAssetsByClass(UZWQuestFact::StaticClass()->GetClassPathName(), ..., true)` (recursive), then `LoadObject<UZWQuestFact>(GetTransientPackage(), *Path)` **into the transient package** for each asset, copying `FactGuid` and `FactName` into a new `FZWQuestFactData`.
- `UFUNCTION(BlueprintCallable, Category="QuestFactBaseSubsystem") TArray<FZWQuestFactData>& GetFacts();` — ⚠️ appends **every** current fact onto member `FactsToPrint` and returns a reference to that accumulating array (see §8).
- `UFUNCTION(BlueprintCallable, Category="QuestFactBaseSubsystem") void SetFactValue(FName inName, int32 inValue);` — linear search by `Name`, sets `Value` on **every** fact with matching name (not just first), broadcasts `OnFactValueChanged` per hit.
- `UFUNCTION(BlueprintCallable, Category="QuestFactBaseSubsystem") int32 GetFactValue(FName inName);` — linear search,; returns last matching fact's value, `0` when not found.
- `TArray<FZWQuestFactData> FactsToPrint;` — accumulator backing `GetFacts`.

### Module 2: `ZWQuestFactBaseEditor` (Editor)

#### `FZWQuestFactBaseEditorModule` — `IModuleInterface`
`ZWQuestFactBaseEditor.h/.cpp`.
- `void StartupModule();`:
  1. Loads `PropertyEditor` module and registers the property customization: `RegisterCustomPropertyTypeLayout(FZWQuestFactSearchableName::StaticStruct()->GetFName(), …CreateStatic(&FZWQuestFactNameCustomization::MakeInstance))`.
  2. Initializes `FZWQuestFactBaseEditorStyle` + `ReloadTextures`.
  3. `FZWQuestFactBaseEditorCommands::Register()`; creates `PluginCommands` and maps `OpenPluginWindow → PluginButtonClicked`.
  4. ToolMenus startup callback → `RegisterMenus()`.
  5. Nomad tab spawner named `"Quest Fact Base Editor"` (display name "QuestFactBaseEditor", hidden menu type), spawning `SNew(SZWQuestFactBaseEditor)`.
- `void ShutdownModule();` — unregisters ToolMenus callback/owner, style shutdown, `Commands::Unregister()`, nomad-tab unregister, and (if `PropertyEditor` loaded) `UnregisterCustomPropertyTypeLayout(FZWQuestFactSearchableName::StaticStruct()->GetFName())`.
- `void RegisterMenus();` — extends `LevelEditor.MainMenu.Window` section `WindowLayout` with the OpenPluginWindow menu entry, and `LevelEditor.LevelEditorToolBar` section `Settings` with a toolbar button; both bound to `PluginCommands`.
- `IMPLEMENT_MODULE(FZWQuestFactBaseEditorModule, ZWQuestFactBaseEditor)`.

#### `FZWQuestFactBaseEditorStyle`
`ZWQuestFactBaseEditorStyle.h/.cpp`. Identical API/handling to `FZWQuestFactBaseStyle` but with `GetStyleSetName() == "QuestFactBaseEditorStyle"` and the same four brushes (`QuestFactBase.OpenPluginWindow`, `QuestFactBase.Fact`, `QuestFactBase.FolderClosed`, `QuestFactBase.FolderOpen`), engine content root. Note `SZWQuestFactBaseEditor` actually reads most brushes from `FAppStyle` (engine), not this set — see `Construct`.

#### `FZWQuestFactBaseEditorCommands` — `TCommands<FZWQuestFactBaseEditorCommands>`
`ZWQuestFactBaseEditorCommands.h/.cpp`. Context: `TEXT("QuestFactBase") / "QuestFactBase Plugin"`, style set from the editor style.
Command infos (all `EUserInterfaceActionType::Button`):
| Command | Label | Description | Chord |
|---|---|---|---|
| `OpenPluginWindow` | "QuestFactBase" | Bring up QuestFactBase window | none |
| `CreateNewFolder` | "QuestFactBase" | Create new Folder | none |
| `CreateNewFact` | "QuestFactBase" | Create new Fact | none |
| `RemoveFact` | "QuestFactBase" | Remove the selected Fact | none |
| `RenameFact` | "QuestFactBase" | Rename the selected Fact | **F2** |
| `ClearSelection` | "QuestFactBase" | Clear the current selection | **Escape** |

> Note: all six commands share the same UI label "QuestFactBase"; labels are collapsed in the toolbar (`SetLabelVisibility(Collapsed)`).

#### `SZWQuestFactTreeRow` — `STableRow<UZWQuestFact*>` (header only usage in `OnGenerateRow` is bypassed)
`SZWQuestFactBaseEditor.h`. SLATE args: default `Content` slot, `SLATE_ARGUMENT(UZWQuestFact*, Entry)`. `Construct(const FArguments&, const TSharedRef<STableViewBase>&)`. Member `UZWQuestFact* Entry = nullptr;`. Declared but **never instantiated** in the current `.cpp` (rows are plain `STableRow` + `SHorizontalBox` + `SImage` + `SInlineEditableTextBlock`).

Typedef: `using SZWQuastFactTreeView` is `typedef STreeView<UZWQuestFact*> SZWQuestFactTreeView;`

#### `SZWQuestFactBaseEditor` — `SCompoundWidget` (`ZWQUESTFACTBASEEDITOR_API`)
The actual editor UI. Constants exposed as data members: `const int MAX_BUTTON_WIDTH = 70;` (unused), `const FString QUESTFACTBASE_FACT_DIR = "/Game/QuestFacts";` (the asset output directory — note: `FPaths::Combine("/Game/QuestFacts", Name)` is used directly as a package name).

**Public members / Slate interface:**
- `void Construct(const FArguments& InArgs);` — builds command list + bindings; `FSlimHorizontalToolBarBuilder` with three buttons (CreateNewFolder icon `ContentBrowser.NewFolderIcon`, CreateNewFact icon `Icons.Plus`, RemoveFact icon `Icons.Minus` — all from `FAppStyle`); `SNew(SZWQuestFactTreeView)` with `Single` selection mode, `ClearSelectionOnClick(true)`, source `&TopLevelItems`, delegates `OnGenerateRow/OnGetChildren/OnSelectionChanged/OnMouseButtonDoubleClick`; `SSearchBox` wired to `OnFilterTextChanged/OnFilterTextCommitted` with `DelayChangeNotificationsWhileTyping(true)` and metadata tag `QuestFactBase.Search`; layout is `SVerticalBox` (toolbar / search / tree). Post-construction it sets expansion for top-level items (twice, with a log line), calls `OnSelectionChanged(nullptr, ESelectInfo::Direct)` and `RebuildTree()`.
- `~SZWQuestFactBaseEditor();` — empties `TopLevelItems` only (raw `UObject*` tree items; they live in the transient package / GC).
- `void BindCommands();` — maps `CreateNewFolder/CreateNewFact/RemoveFact/RenameFact/ClearSelection` to the respective widget methods with `FCanExecuteAction()` (always-executable).
- `void FilterView(const FString& InFilter);` — stores filter string into `CurrentFilter` then `RebuildTree()`.
- `void OnFilterTextChanged(const FText& Text);` → `FilterView`.
- `void OnFilterTextCommitted(const FText&, ETextCommit::Type);` — on `OnCleared`: clears search box + filter, clears keyboard focus.
- Tree delegates: `TSharedRef<ITableRow> OnGenerateRow(UZWQuestFact*, const TSharedRef<STableViewBase>&)` (null item → text "THIS WAS NULL SOMEHOW"; otherwise icon column (max width 16) + `SInlineEditableTextBlock` cached in `CachedInLineEditors` with `OnTextCommitted`); `void OnGetChildren(UZWQuestFact*, TArray<UZWQuestFact*>&)` — appends `Item->SubFacts`; `void OnSelectionChanged(UZWQuestFact*, ESelectInfo::Type)` — enables/disables the toolbar Remove button by locating it via a hard-coded 7-level `ChildSlot` child-chain (fragile, see §8); `void OnDoubleClicked(UZWQuestFact*)` — enters inline rename; `void OnTextCommitted(const FText&, ETextCommit::Type)` — writes `FactName` via `FObjectEditorUtils::SetPropertyValue`, prompts checkout+save of the object's package (`FEditorFileUtils::PromptForCheckoutAndSave({Package}, false, false, FText(), FText(), nullptr, false, false)`), regenerates the row from the stashed `Geometry` and rebuilds the tree.
- `const FSlateBrush* GetEntryBrush(UZWQuestFact* Item);` — `QuestFactBase.FolderClosed` for folders, `QuestFactBase.Fact` otherwise (from the editor style set).
- CRUD: `void CreateNewQuestFolder();`, `void CreateNewQuestFact();`, `void RemoveQuestFact();`, `void RenameQuestFact();`, `void ClearCurrentSelection();` (empty), `void RemoveFact(UZWQuestFact*)` (empty stub), `void RebuildTree();`
- Members: `TSharedPtr<SSearchBox> SearchBox;` `TSharedPtr<SZWQuestFactTreeView> SQuestFactTreeViewPtr;` `TArray<UZWQuestFact*> TopLevelItems;` `TArray<UZWQuestFact*> FactsToRebuild;` `TMap<UZWQuestFact*, TSharedPtr<SInlineEditableTextBlock>> CachedInLineEditors;` `TSharedPtr<FUICommandList> CommandList;` `UZWQuestFact* QueuedRenameRequest = nullptr;` `FGeometry Geometry;` — all public (protected only `FString CurrentFilter;`).

**Create/rebuild semantics (from `SZWQuestFactBaseEditor.cpp`):**
- `CreateNewQuestFact` / `CreateNewQuestFolder` (mirrored logic): new `FGuid` → asset (object) name = GUID in `DigitsWithHyphensLower` format; package name `FPaths::Combine("/Game/QuestFacts", <guid>)`; `CreatePackage`, `NewObject<UZWQuestFolder|UZWQuestFact>(Package, …, RF_Public|RF_Standalone|RF_Transactional)` via `FSavePackageArgs.TopLevelFlags`; sets `FactGuid` and default `FactName` (`"New Folder"` / `"New Fact"`); if exactly one item is selected: child is parented to the selected folder's GUID, or inherits the selection's own `ParentId`; `PromptForCheckoutAndSave({Package}, …)`; `AssetRegistry.AssetCreated(...)`; `RebuildTree()`; selects the new item.
- `RemoveQuestFact`: collects all selected items **and recursively their entire descendant sets** (reversed with `Algo::Reverse`), then deletes each via `UEditorAssetLibrary::DeleteAsset(GetPathName())` under a `FScopedSlowTask`; errors out (`UE_LOG Error`) when nothing is selected.
- `RebuildTree`: empties state; re-queries the Asset Registry (`GetAssetsByClass`, recursive), `LoadObject` into `GetTransientPackage()`; drops facts whose `FactName` does not contain `CurrentFilter`; clears every fact's `SubFacts`; parenting pass — for every fact with valid `ParentId` (only when the filter is empty), finds the folder whose `FactGuid == ParentId` and adds the fact as its `SubFact` (`AddUnique`), otherwise fact surfaces as `TopLevelItem`. Ends with `RequestTreeRefresh()`. Filter mode therefore flattens matched facts to top level.

#### `FZWQuestFactNameCustomization` — `IPropertyTypeCustomization` (`ZWQUESTFACTBASEEDITOR_API`)
Detail-panel customization for `FZWQuestFactSearchableName`, giving a searchable dropdown of fact names.
- `static TSharedRef<IPropertyTypeCustomization> MakeInstance();`
- `void CustomizeHeader(TSharedRef<IPropertyHandle>, FDetailWidgetRow&, IPropertyTypeCustomizationUtils&) override;`
  - Reads the `GetOptions` metadata from the property handle; resolves the owner object (`GetOuterObjects()[0]`), `FindFunction(FName(*FunctionName))`, validates `NumParms == 1 && GetReturnProperty()`, invokes it with `ProcessEvent(Func, &ResultArray)` expecting a `TArray<FName>`, and fills `OptionsSource`.
  - Builds header row: property name widget + `SComboButton` whose menu content is `SVerticalBox` = `SEditableTextBox` (search) + `SListView<TSharedPtr<FString>>` (options); button content is an `STextBlock` bound to `GetCurrentSelectionText()`; `OnMenuOpenChanged` resets the search box and re-sources the full option list on open.
- `void CustomizeChildren(...)` — empty.
- Private: `void OnSearchTextChanged(const FText&)` (substring filter over `OptionsSource` into `FilteredOptions`, `RequestListRefresh`); `void OnSelectionChanged(TSharedPtr<FString>, ESelectInfo::Type)` — writes the choice into the child property `QuestFactName` via `SetValue(FName(**NewSelection))` and closes the combo; `TSharedRef<ITableRow> OnGenerateRow(TSharedPtr<FString>, const TSharedRef<STableViewBase>&)` (plain `STableRow` + `STextBlock`); `FText GetCurrentSelectionText() const;` (reads current `QuestFactName` from child handle); `void OnMenuOpenChanged(bool bIsOpen);`
- Members: `TSharedPtr<IPropertyHandle> PropertyHandlePtr;` `TArray<TSharedPtr<FString>> OptionsSource; FilteredOptions;` `TSharedPtr<SListView<TSharedPtr<FString>>> ListView;` `TSharedPtr<SEditableTextBox> SearchBar;` `TSharedPtr<SComboButton> ComboButton;`

## 5. Implementacja (Private) — podsumowanie plików

Runtime module privates (`Source/ZWQuestFactBase/Private/`):
| File | Content |
|---|---|
| `ZWQuestFact.cpp` | `GetPrimaryAssetId`, empty `UZWQuestFact()` ctor, `GetGuid`, `IsFolder`, `UZWQuestFolder` ctor (`bIsFolder = true`). |
| `ZWQuestFactBase.cpp` | Module startup/shutdown + nomad tab (see `FZWQuestFactBaseModule`). Loctext ns `FZWQuestFactBaseModule`. |
| `ZWQuestFactBaseStyle.cpp` | Slate style set (see class section). |
| `ZWQuestFactBaseSubsystem.cpp` | Asset-registry fact registration + value read/write + delegate broadcast (see class section). |

Editor module privates (`Source/ZWQuestFactBaseEditor/Private/`):
| File | Content |
|---|---|
| `ZWQuestFactBaseEditor.cpp` | Module startup (property customization registration first, then style/commands/menus/tab). |
| `ZWQuestFactBaseEditorStyle.cpp` | Style set "QuestFactBaseEditorStyle" (identical brushes). |
| `ZWQuestFactBaseEditorCommands.cpp` | Six `UI_COMMAND`s (table above). Loctext ns "FQuestFactBaseModule". |
| `SZWQuestFactBaseEditor.cpp` | Full widget logic: toolbar, search, tree, CRUD, rename, delete cascade, tree rebuild (see class section). Wrapped in `BEGIN/END_SLATE_FUNCTION_BUILD_OPTIMIZATION`. |
| `ZWQuestFactNameCustomization.cpp` | GetOptions-driven searchable combo (see class section). |

Editor widget internals worth noting:
- Rename flow:ToolBarBuilders buttons have no text labels (labels collapsed) — commands are identified purely by icons plus tooltips left empty (`TAttribute<FText>()` default); `RenameQuestFact` and `OnDoubleClicked` share the inline-edit path via `QueuedRenameRequest` and the cached geometry (`GetTickSpaceGeometry()`), which is later passed to `ReGenerateItems`.
- The Remove toolbar button enable/disable uses `ChildSlot.GetChildAt(0)->GetChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(2)` — position-dependent child traversal (see §8).
- `BindCommands` uses `CreateSP` (widget-lifetime-safe) for the five widget actions; the module's `OpenPluginWindow` mapping uses `CreateRaw` on the module.
- The commented-out `UPackage::Save` call indicates intent to save packages directly; the active path instead relies on `PromptForCheckoutAndSave` (may silently require user confirmation in the save dialog).

## 6. Konfiguracja (.ini)

`Config/FilterPlugin.ini` — contains only the `[FilterPlugin]` section with the stock comment block (paths-relative packaging wildcards helper); **no actual entries**. Effectively unused.

No other `.ini` config is present. All configuration is hard-coded in C++:
- Fact asset output dir: `/Game/QuestFacts` (`QUESTFACTBASE_FACT_DIR` in `SZWQuestFactBaseEditor.h`).
- Default fact value: `0` (`DefaultFactValue` in `ZWQuestFactBaseSubsystem.h`).
- Engine Slate content root for icons; no plugin-owned icon resources are referenced by code (the unused `Resources/Icon128.png` / `PlaceholderButtonIcon.svg` remain packaged).

## 7. Zależności wewnątrz ZWSuite

- `ZWQuestFactBaseEditor.Build.cs` publicly declares `"ZWQuestFactBase"` — the only intra-plugin (and intra-ZWSuite-shared namespace) dependency documented here.
- No other `ZW*` modules or headers are included; the plugin is self-contained apart from engine modules (`Core`, `CoreUObject`, `Engine`, `Slate(SlateCore)`, `Projects`, `InputCore`, `ToolMenus`, `AssetRegistry`, `PropertyEditor`, `UnrealEd`, `EditorFramework`, `EditorScriptingUtilities`, `FileHelpers`, `EditorAssetLibrary`, `ObjectEditorUtils`, `SavePackage`, `Starship`-engine Slate assets for icons).
- External plugin dependency (`.uplugin`): `EditorScriptingUtilities` (enabled). Also referenced directly in both Build.cs files.
- Facts are consumed via `UZWQuestFactBaseSubsystem` from any `GameInstance`; the subsystem registers from the Asset Registry classes list, so any Blueprints-derived `UZWQuestFact` subclasses in the project are also discovered (GetAssetsByClass is recursive `true`).
- Cross-plugin consumers (e.g., other ZWSuite plugins) would bind to `OnFactValueChanged` / call `GetFactValue` / `SetFactValue` — no such consumer is present in this source tree (brak in-tree consumers).

## 8. Uwagi / ryzyka (Notes / Risks)

1. **`GetFacts()` accumulates unboundedly** — every call appends the full `QuestFacts` list onto `FactsToPrint` (duplicates grow on repeated calls; the member is never cleared). Blueprint consumers will see stale duplicated data.
2. **Raw `new` / deleted FZWQuestFactData** — `QuestFacts` owns `FZWQuestFactData*` allocated with `new`; there is no dtor cleanup (`Deinitialize` is empty) → leak per GameInstance, and the subsystem holds dangling pointers if not torn down before facts.
3. **Non-UPROPERTY trees** — `UZWQuestFact::SubFacts` and `FZWQuestFactData::Parent/Children` are plain C++ members: not serialized, not reflected, not traced by GC. `Parent`/`Children` are never populated at all.
4. **Fact assets load into the transient package** — both `RegisterFacts()` (subsystem) and `RebuildTree()` use `LoadObject<UZWQuestFact>(GetTransientPackage(), …)`. `bIsFolder` is `const`-upcast-safe here, but this deserialization path means runtime state is a copy; each asset is loaded per rebuild (performance grows with fact count), and transient-package copies of `UObject`s can interact poorly with GC.
5. **`SetFactValue`/`GetFactValue` match by `FName`, not by GUID** — duplicate fact names across the tree all get set / report the same value; identity is ambiguous. `GetFactValue` returns the *last* matching fact's value (loop picks every hit), and `0` for not-found is indistinguishable from a fact whose value is genuinely 0.
6. **Fragile toolbar child traversal** — `OnSelectionChanged` reaches the Remove button via hard-coded `GetChildAt(...)` chains; any layout change silently breaks enable/disable.
7. **`SZXQuestFactTreeRow` (a.k.a. `SZWQuestFactTreeRow`) is dead** — declared but never constructed; the row generator doesn't use `Entry`. Similarly the `FolderOpen` brush and plugin's own `Resources/` icons are unused (only `FolderClosed` is queried).
8. **Editor-side class names with obvious typos still compile** — e.g. typedef is correct (`typedef STreeView<UZWQuestFact*> SZWQuestFactTreeView;`) but this member does not expose rename, delete, or folder visibility beyond what its methods provide.
9. **Nomad tab in the runtime module** spawns a bare `SButton` — leftover template code; `RegisterMenus()` (runtime module) is empty and `PluginCommands` unused. The runtime module also pulls in `UnrealEd/EditorFramework` in editor targets, which a purely runtime plugin shouldn't need.
10. **Asset name = GUID string** — creation misses uniqueness checks if a GUID collision ever occurs (no `FPackageName::SuggestName`/"MakeName" retry); package path derives from `FPaths::Combine("/Game/QuestFacts", Guid)` without checking a leading "/" root policy.
11. **`FZWQuestFactSearchableName` customization requires `GetOptions` metadata** on the property — otherwise the combo shows an empty option list (still searchable but a no-op). Only the child handle `QuestFactName` is written; the wrapper struct's owner value is updated through the child.
12. **Deleting a folder does not reparent** surviving facts (their `ParentId` still points at a dead GUID) — after folder deletion, orphaned facts will nevertheless disappear from the *filtered* tree (filter mode drops invalid-parent handling) but will keep existing as orphan top-level items otherwise.

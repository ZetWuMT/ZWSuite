 // Fill out your copyright notice in the Description page of Project Settings.


#include "SZWQuestFactBaseEditor.h"
#include "ZWQuestFactBaseEditorCommands.h"
#include "ZWQuestFactBaseEditorStyle.h"
#include "SlateOptMacros.h"
#include "ZWQuestFact.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/SavePackage.h"
#include "ObjectEditorUtils.h"
#include "FileHelpers.h"
#include "EditorAssetLibrary.h"
#include "Widgets/Input/SSearchBox.h"


#define LOCTEXT_NAMESPACE "QuestFactBaseEditor"

 BEGIN_SLATE_FUNCTION_BUILD_OPTIMIZATION

 void SZWQuestFactBaseEditor::Construct(const FArguments& InArgs)
{
	CommandList = MakeShareable(new FUICommandList);

	BindCommands();

	FSlimHorizontalToolBarBuilder ToolBarBuilder(CommandList, FMultiBoxCustomization::None);
	ToolBarBuilder.SetLabelVisibility(EVisibility::Collapsed);
	ToolBarBuilder.AddToolBarButton(FZWQuestFactBaseEditorCommands::Get().CreateNewFolder, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FAppStyle::Get().GetStyleSetName(), TEXT("ContentBrowser.NewFolderIcon")));
	ToolBarBuilder.AddToolBarButton(FZWQuestFactBaseEditorCommands::Get().CreateNewFact, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FAppStyle::Get().GetStyleSetName(), TEXT("Icons.Plus")));
	ToolBarBuilder.AddToolBarButton(FZWQuestFactBaseEditorCommands::Get().RemoveFact, NAME_None, TAttribute<FText>(), TAttribute<FText>(), FSlateIcon(FAppStyle::Get().GetStyleSetName(), TEXT("Icons.Minus")));

	// clang-format off

	SQuestFactTreeViewPtr =
		SNew(SZWQuestFactTreeView)
		//.ItemHeight(24)
		.SelectionMode(ESelectionMode::Single)
		.ClearSelectionOnClick(true)
		.TreeItemsSource(&TopLevelItems)
		.OnGenerateRow(this, &SZWQuestFactBaseEditor::OnGenerateRow)
		.OnGetChildren(this, &SZWQuestFactBaseEditor::OnGetChildren)
		.OnSelectionChanged(this, &SZWQuestFactBaseEditor::OnSelectionChanged)
		.OnMouseButtonDoubleClick(this, &SZWQuestFactBaseEditor::OnDoubleClicked);

	ChildSlot
		[
			SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					ToolBarBuilder.MakeWidget()
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SAssignNew(SearchBox, SSearchBox)
					.HintText(LOCTEXT("SearchDetailsHint", "Search"))
					.OnTextChanged(this, &SZWQuestFactBaseEditor::OnFilterTextChanged)
					.OnTextCommitted(this, &SZWQuestFactBaseEditor::OnFilterTextCommitted)
					.DelayChangeNotificationsWhileTyping(true)
					.AddMetaData<FTagMetaData>(TEXT("QuestFactBase.Search"))
				]
				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SQuestFactTreeViewPtr.ToSharedRef()
				]
		];

	//clang-format on

	for (auto RootFactIt(TopLevelItems.CreateConstIterator()); RootFactIt; ++RootFactIt)
	{
		const auto& Fact = *RootFactIt;
		SQuestFactTreeViewPtr->SetItemExpansion(Fact, true);
	}

	for (UZWQuestFact* Fact : TopLevelItems)
	{
		SQuestFactTreeViewPtr->SetItemExpansion(Fact, true);
		UE_LOG(LogTemp, Log, TEXT("Item %s expanded"), *(Fact->FactName).ToString())
	}

	OnSelectionChanged(nullptr, ESelectInfo::Direct);

	RebuildTree();
}

SZWQuestFactBaseEditor::~SZWQuestFactBaseEditor()
{
	TopLevelItems.Empty();
}

void SZWQuestFactBaseEditor::BindCommands()
{
	const FZWQuestFactBaseEditorCommands& Commands = FZWQuestFactBaseEditorCommands::Get();

	CommandList->MapAction(
		Commands.CreateNewFolder,
		FExecuteAction::CreateSP(this, &SZWQuestFactBaseEditor::CreateNewQuestFolder),
		FCanExecuteAction());
	CommandList->MapAction(
		Commands.CreateNewFact,
		FExecuteAction::CreateSP(this, &SZWQuestFactBaseEditor::CreateNewQuestFact),
		FCanExecuteAction());
	CommandList->MapAction(
		Commands.RemoveFact,
		FExecuteAction::CreateSP(this, &SZWQuestFactBaseEditor::RemoveQuestFact),
		FCanExecuteAction());
	CommandList->MapAction(
		Commands.RenameFact,
		FExecuteAction::CreateSP(this, &SZWQuestFactBaseEditor::RenameQuestFact),
		FCanExecuteAction());
	CommandList->MapAction(
		Commands.ClearSelection,
		FExecuteAction::CreateSP(this, &SZWQuestFactBaseEditor::ClearCurrentSelection),
		FCanExecuteAction());
}

 void SZWQuestFactBaseEditor::FilterView(const FString& InFilter)
 {
 	//bool bHadActiveFilter = CurrentFilter.FilterStrings.Num() > 0;

 	CurrentFilter.Empty();
 	CurrentFilter.Append(InFilter);
 	//FString CurrentFilterString;

 	//FString ParseString = InFilter;
 	// Remove whitespace from the front and back of the string
 	//ParseString.TrimStartAndEndInline();
 	//ParseString.ParseIntoArray(CurrentFilterStrings, TEXT(" "), true);

 	RebuildTree();

 }

 void SZWQuestFactBaseEditor::OnFilterTextChanged(const FText& InFilterText)
 {
 	FilterView(InFilterText.ToString());
 }

void SZWQuestFactBaseEditor::OnFilterTextCommitted(const FText& Text, ETextCommit::Type InCommitType)
 {
 	if (InCommitType == ETextCommit::OnCleared)
 	{
 		SearchBox->SetText(FText::GetEmpty());
 		OnFilterTextChanged(FText::GetEmpty());
 		FSlateApplication::Get().ClearKeyboardFocus(EFocusCause::Cleared);
 	}
 }

TSharedRef<ITableRow> SZWQuestFactBaseEditor::OnGenerateRow(UZWQuestFact* Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	//clang-format off	
	
	if (!Item)
	{
		return SNew(STableRow<UZWQuestFact*>, OwnerTable)
			[
				SNew(STextBlock)
					.Text(FText::FromString("THIS WAS NULL SOMEHOW"))
			];
	}

	return SNew(STableRow<UZWQuestFact*>, OwnerTable)
		[
			SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.Padding(5, 1)
				.MaxWidth(16)
				[
					SNew(SImage)
						.Image(GetEntryBrush(Item))
				]
				+ SHorizontalBox::Slot()
				[
					SAssignNew(CachedInLineEditors.FindOrAdd(Item), SInlineEditableTextBlock)
						.Text(FText::FromName(Item->FactName))
						.OnTextCommitted(this, &SZWQuestFactBaseEditor::OnTextCommitted)
				]			
		];

	//clang-format on	
}

const FSlateBrush* SZWQuestFactBaseEditor::GetEntryBrush(UZWQuestFact* Item)
{
	if (Item->IsFolder())
	{
		return FZWQuestFactBaseEditorStyle::Get().GetBrush("QuestFactBase.FolderClosed");
	}
	return FZWQuestFactBaseEditorStyle::Get().GetBrush("QuestFactBase.Fact");
}

void SZWQuestFactBaseEditor::OnGetChildren(UZWQuestFact* Item, TArray<UZWQuestFact*>& OutChildren)
{
	if (!Item->IsValidLowLevelFast())
	{
		return;
	}
	const TArray<UZWQuestFact*>& SubFacts = Item->SubFacts;
	OutChildren.Append(SubFacts);
}

void SZWQuestFactBaseEditor::OnSelectionChanged(UZWQuestFact* Item, ESelectInfo::Type SelectInfo)
{	
	SWidget& RemoveToolBarButton = this->ChildSlot.GetChildAt(0)->GetChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(2).Get();

	if (SQuestFactTreeViewPtr->GetSelectedItems().Num() != 0)
	{
		RemoveToolBarButton.SetEnabled(true);
	}
	else
	{
		RemoveToolBarButton.SetEnabled(false);
	}
}

void SZWQuestFactBaseEditor::RenameQuestFact()
{
	if (SQuestFactTreeViewPtr->GetSelectedItems().Num() != 0)
	{
		QueuedRenameRequest = SQuestFactTreeViewPtr->GetSelectedItems()[0];

		//we need to store the textblock geometry so we can then regenerate the entry in OnTextCommitted
		Geometry = CachedInLineEditors[QueuedRenameRequest]->GetTickSpaceGeometry();

		CachedInLineEditors[QueuedRenameRequest]->EnterEditingMode();
	}	
}

void SZWQuestFactBaseEditor::OnDoubleClicked(UZWQuestFact* Item)
{
	QueuedRenameRequest = Item;	

	//we need to store the textblock geometry so we can then regenerate the entry in OnTextCommitted
	Geometry = CachedInLineEditors[QueuedRenameRequest]->GetTickSpaceGeometry();

	CachedInLineEditors[QueuedRenameRequest]->EnterEditingMode();
}

void SZWQuestFactBaseEditor::OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType)
{
	if (InCommitType == ETextCommit::OnCleared || QueuedRenameRequest == nullptr)
	{
		return;
	}

	static const FName NamePropertyName = GET_MEMBER_NAME_CHECKED(UZWQuestFact, FactName);
	FObjectEditorUtils::SetPropertyValue(QueuedRenameRequest, NamePropertyName, FName(InText.ToString()));
	FEditorFileUtils::PromptForCheckoutAndSave({ QueuedRenameRequest->GetPackage() }, false, false, FText(), FText(), nullptr, false, false);
	
	QueuedRenameRequest = nullptr;

	SQuestFactTreeViewPtr->ReGenerateItems(Geometry);
	RebuildTree();
}



void SZWQuestFactBaseEditor::CreateNewQuestFolder()
{
	// Load necessary modules
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Generate a unique asset name

	FGuid FactGuid(FGuid::NewGuid());
	const FString Name = FactGuid.ToString(EGuidFormats::DigitsWithHyphensLower);
	const FString PackageName = FPaths::Combine(QUESTFACTBASE_FACT_DIR, Name);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

	// Create FSavePackageArgs for saving the package	

	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone | RF_Transactional;

	// Create object and package
	UPackage* Package = CreatePackage(*PackageName);
	UZWQuestFolder* NewFolder = NewObject<UZWQuestFolder>(Package, UZWQuestFolder::StaticClass(), FName(Name), SaveArgs.TopLevelFlags);

	NewFolder->FactGuid = FactGuid;
	NewFolder->FactName = FName("New Folder");

	if (SQuestFactTreeViewPtr->GetSelectedItems().Num() == 1)
	{
		UZWQuestFact* CurrentSelection = SQuestFactTreeViewPtr->GetSelectedItems()[0];

		if (CurrentSelection->IsFolder())
		{
			NewFolder->ParentId = CurrentSelection->FactGuid;
			SQuestFactTreeViewPtr->SetItemExpansion(CurrentSelection, true);
		}	
		else if (CurrentSelection->ParentId.IsValid())
		{
			NewFolder->ParentId = CurrentSelection->ParentId;
		}
	}

	//FSavePackageResultStruct saveResult = UPackage::Save(Package, NewFolder, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);
	FEditorFileUtils::PromptForCheckoutAndSave({ Package }, false, false, FText(), FText(), nullptr, false, false);

	// Inform asset registry
	AssetRegistry.AssetCreated(NewFolder);

	RebuildTree();

	SQuestFactTreeViewPtr->SetSelection(NewFolder);

	//UE_LOG(LogTemp, Warning, TEXT("Asset not created"))
}

void SZWQuestFactBaseEditor::CreateNewQuestFact()
{
	// Load necessary modules
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Generate a unique asset name
	
	FGuid FactGuid(FGuid::NewGuid());
	const FString Name = FactGuid.ToString(EGuidFormats::DigitsWithHyphensLower);
	const FString PackageName = FPaths::Combine(QUESTFACTBASE_FACT_DIR, Name);
	const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

	// Create FSavePackageArgs for saving the package	
	
	FSavePackageArgs SaveArgs;
	SaveArgs.TopLevelFlags = RF_Public | RF_Standalone | RF_Transactional;

	// Create object and package
	UPackage* Package = CreatePackage(*PackageName);
	UZWQuestFact* NewFact = NewObject<UZWQuestFact>(Package, UZWQuestFact::StaticClass(), FName(Name), SaveArgs.TopLevelFlags);

	NewFact->FactGuid = FactGuid;
	NewFact->FactName = FName("New Fact");

	if (SQuestFactTreeViewPtr->GetSelectedItems().Num() == 1)
	{
		UZWQuestFact* CurrentSelection = SQuestFactTreeViewPtr->GetSelectedItems()[0];

		if (CurrentSelection->IsFolder())
		{
			NewFact->ParentId = CurrentSelection->FactGuid;
			SQuestFactTreeViewPtr->SetItemExpansion(CurrentSelection, true);
		}
		else if (CurrentSelection->ParentId.IsValid())
		{
			NewFact->ParentId = CurrentSelection->ParentId;
		}
	}

	//FSavePackageResultStruct saveResult = UPackage::Save(Package, NewFact, *FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension()), SaveArgs);
	FEditorFileUtils::PromptForCheckoutAndSave({ Package }, false, false, FText(), FText(), nullptr, false, false);

	// Inform asset registry
	AssetRegistry.AssetCreated(NewFact);

	RebuildTree();

	SQuestFactTreeViewPtr->SetSelection(NewFact);

	//UE_LOG(LogTemp, Warning, TEXT("Asset not created"))
}

void SZWQuestFactBaseEditor::RemoveQuestFact()
{
	// Load necessary modules
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray <UZWQuestFact*> SelectedItems = SQuestFactTreeViewPtr->GetSelectedItems();
	TArray <UZWQuestFact*> AssetsToDelete;

	if (SelectedItems.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("Removing fact failed! No fact is selected."))

		return;
	}

	for (UZWQuestFact* InCurrentSelection : SelectedItems)
	{
		AssetsToDelete.Add(InCurrentSelection);

		if (!InCurrentSelection->SubFacts.IsEmpty())
		{
			TArray<UZWQuestFact*> FactsToCheck = InCurrentSelection->SubFacts;

			while (FactsToCheck.Num())
			{
				AssetsToDelete.Add(FactsToCheck[0]);
				for (UZWQuestFact* SubFact : FactsToCheck[0]->SubFacts)
				{
					FactsToCheck.Add(SubFact);
				}
				FactsToCheck.RemoveAt(0);
			}
		}		
	}	
	
	Algo::Reverse(AssetsToDelete);

	FScopedSlowTask SlowTask(AssetsToDelete.Num(), LOCTEXT("DeletingFacts", "Deleting fact and any children..."));
	for (UZWQuestFact* AssetToDelete : AssetsToDelete)
	{
		SlowTask.EnterProgressFrame();
		UEditorAssetLibrary::DeleteAsset(AssetToDelete->GetPathName());
	}

	RebuildTree();
}

void SZWQuestFactBaseEditor::ClearCurrentSelection()
{

}

void SZWQuestFactBaseEditor::RemoveFact(UZWQuestFact* Fact)
{

}


void SZWQuestFactBaseEditor::RebuildTree()
{
	TopLevelItems.Empty();
	FactsToRebuild.Empty();
	
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> UQuestFactDataArray;
	AssetRegistry.GetAssetsByClass(UZWQuestFact::StaticClass()->GetClassPathName(), UQuestFactDataArray, true);	

	for (FAssetData UQuestFactData : UQuestFactDataArray)
	{
		FString Path = UQuestFactData.GetObjectPathString();
		UZWQuestFact* Fact = LoadObject<UZWQuestFact>(GetTransientPackage(), *Path);
		if (!CurrentFilter.IsEmpty() && !Fact->FactName.ToString().Contains(CurrentFilter)) continue;
		FactsToRebuild.Add(Fact);		
	}

	for (UZWQuestFact* Fact : FactsToRebuild)
	{
		Fact->SubFacts.Empty();
	}

	for (UZWQuestFact* Fact : FactsToRebuild)
	{		
		if (CurrentFilter.IsEmpty() && Fact->ParentId.IsValid())
		{
			for (UZWQuestFact* ParentFact : FactsToRebuild)
			{
				if (ParentFact->IsFolder() && ParentFact->FactGuid == Fact->ParentId)
				{
					ParentFact->SubFacts.AddUnique(Fact);
				}
			}
		}
		else
		{
			TopLevelItems.Add(Fact);
		}
	}

	if (SQuestFactTreeViewPtr.IsValid())
	{
		SQuestFactTreeViewPtr->RequestTreeRefresh();
	}
}

#undef LOCTEXT_NAMESPACE

END_SLATE_FUNCTION_BUILD_OPTIMIZATION

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "ZWQuestFact.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"

/**
 * 
 */


class SZWQuestFactTreeRow : public STableRow<UZWQuestFact*>
{
public:
	SLATE_BEGIN_ARGS(SZWQuestFactTreeRow) {}
		SLATE_DEFAULT_SLOT(typename SZWQuestFactTreeRow::FArguments, Content)
		SLATE_ARGUMENT(UZWQuestFact*, Entry)
	SLATE_END_ARGS()

	using SSuperRowType = STableRow<UZWQuestFact*>;

	void Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView);

private:
	UZWQuestFact* Entry = nullptr;
};

typedef STreeView <UZWQuestFact*> SZWQuestFactTreeView;

class ZWQUESTFACTBASEEDITOR_API SZWQuestFactBaseEditor : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SZWQuestFactBaseEditor)
	{}
	SLATE_END_ARGS()

	const int MAX_BUTTON_WIDTH = 70;

	const FString QUESTFACTBASE_FACT_DIR = "/Game/QuestFacts";

	/** Constructs this widget with InArgs */
	void Construct(const FArguments& InArgs);

	~SZWQuestFactBaseEditor();

	void BindCommands();
	
	void FilterView(const FString& InFilter);
	//void UpdateFilteredDetails();
	void OnFilterTextChanged(const FText& Text);
	void OnFilterTextCommitted(const FText& Text, ETextCommit::Type Arg);
	
	// TreeView Delegates
	TSharedRef<ITableRow> OnGenerateRow(UZWQuestFact* Item, const TSharedRef<STableViewBase>& OwnerTable);
	void OnGetChildren(UZWQuestFact* Item, TArray<UZWQuestFact*>& OutChildren);
	void OnSelectionChanged(UZWQuestFact* Item, ESelectInfo::Type SelectInfo);
	void OnDoubleClicked(UZWQuestFact* Item);
	void OnTextCommitted(const FText& InText, ETextCommit::Type InCommitType);

	const FSlateBrush* GetEntryBrush(UZWQuestFact* Item);

	void CreateNewQuestFolder();
	void CreateNewQuestFact();
	void RemoveQuestFact();
	void RenameQuestFact();
	void ClearCurrentSelection();

	void RemoveFact(UZWQuestFact* Fact);

	void RebuildTree();

	TSharedPtr<SSearchBox> SearchBox;

	TSharedPtr<SZWQuestFactTreeView> SQuestFactTreeViewPtr;

	TArray<UZWQuestFact*> TopLevelItems;
	TArray<UZWQuestFact*> FactsToRebuild;

	TMap<UZWQuestFact*, TSharedPtr<SInlineEditableTextBlock>> CachedInLineEditors;

	TSharedPtr<FUICommandList> CommandList = nullptr;

	UZWQuestFact* QueuedRenameRequest = nullptr;
	FGeometry Geometry;
	//const SWidget& RemoveToolBarButton;// this->ChildSlot.GetChildAt(0)->GetChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(0)->GetAllChildren()->GetChildAt(2).Get();

protected:
	FString CurrentFilter;
};

// Fill out your copyright notice in the Description page of Project Settings.

#include "QuestChoiceSubsystem.h"
#include "QuestChoicePanelWidgetData.h"

void UQuestChoiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UZWMovieSceneDialogueSubsystem* DialogueSubsystem = Collection.InitializeDependency<UZWMovieSceneDialogueSubsystem>();
    DialogueSubsystem->RegisterDialogueHandler(this);
}

uint32 UQuestChoiceSubsystem::GetOrder() const
{
    return 11u;
}

EZWStartDialogueResult UQuestChoiceSubsystem::OnStartDialogueLine(const FZWDialogueData& DialogueData)
{
    LastDialogueLine = DialogueData;
    return EZWStartDialogueResult::Handled;
}

void UQuestChoiceSubsystem::OnFinishDialogueLine(const FZWDialogueData& DialogueData)
{
}

void UQuestChoiceSubsystem::ChooseOption(const FGuid& NodeGuid, FName Choice)
{
    if (WasOptionChosen(NodeGuid, Choice))
    {
        return;
    }
    ChosenOptions.AddUnique(NodeGuid, Choice);
}

bool UQuestChoiceSubsystem::WasOptionChosen(const FGuid& NodeGuid, FName Choice)
{
    if (ChosenOptions.FindPair(NodeGuid, Choice) != nullptr)
    {
        return true;
    }
    return false;
}

void UQuestChoiceSubsystem::OnShowChoiceDialogueLine(const FGuid& ChoiceSectionID, const FZWDialogueData& DialogueData)
{
    LastChoiceDialogueLine.Key = ChoiceSectionID;
    LastChoiceDialogueLine.Value = DialogueData;
    LastDialogueLine = {};
}

const FZWDialogueData& UQuestChoiceSubsystem::GetDialogueLineToShowDuringChoice(const FGuid& ChoiceSectionID) const
{
    if (LastChoiceDialogueLine.Key == ChoiceSectionID)
    {
        LastChoiceDialogueLine.Value;
    }
    return LastDialogueLine;
}

void UQuestChoiceSubsystem::SetPanelWidgetData(const TObjectPtr<UQuestChoicePanelWidgetData>& ChoiceData)
{
    ChoiceStarted.Broadcast(ChoiceData);
}

void UQuestChoiceSubsystem::ReceiveDataFromQuestChoicePanelWidget(UQuestChoiceChangeableObject* ChoiceData)
{
    UQuestChoicePanelWidgetData* QuestChoicePanelWidgetData = Cast<UQuestChoicePanelWidgetData>(ChoiceData);
}

void UQuestChoiceSubsystem::ResetChosenOptions()
{
    ChosenOptions.Empty();
}

void UQuestChoiceSubsystem::ResetLastDialogueLines()
{
    LastDialogueLine = {};
    LastChoiceDialogueLine = {};
}
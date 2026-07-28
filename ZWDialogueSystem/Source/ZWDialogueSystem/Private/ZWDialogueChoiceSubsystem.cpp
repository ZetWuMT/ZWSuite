// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWDialogueChoiceSubsystem.h"
#include "UI/ZWDialogueChoicePanelWidgetData.h"
#include "ZWUISubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"

void UZWDialogueChoiceSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    UZWMovieSceneDialogueSubsystem* DialogueSubsystem = Collection.InitializeDependency<UZWMovieSceneDialogueSubsystem>();
    DialogueSubsystem->RegisterDialogueHandler(this);
}

uint32 UZWDialogueChoiceSubsystem::GetOrder() const
{
    return 11u;
}

EZWStartDialogueResult UZWDialogueChoiceSubsystem::OnStartDialogueLine(const FZWDialogueData& DialogueData)
{
    LastDialogueLine = DialogueData;
    return EZWStartDialogueResult::Handled;
}

void UZWDialogueChoiceSubsystem::OnFinishDialogueLine(const FZWDialogueData& DialogueData)
{
}

void UZWDialogueChoiceSubsystem::ChooseOption(const FGuid& NodeGuid, FName Choice)
{
    if (WasOptionChosen(NodeGuid, Choice))
    {
        return;
    }
    ChosenOptions.AddUnique(NodeGuid, Choice);
}

bool UZWDialogueChoiceSubsystem::WasOptionChosen(const FGuid& NodeGuid, FName Choice)
{
    if (ChosenOptions.FindPair(NodeGuid, Choice) != nullptr)
    {
        return true;
    }
    return false;
}

void UZWDialogueChoiceSubsystem::OnShowChoiceDialogueLine(const FGuid& ChoiceSectionID, const FZWDialogueData& DialogueData)
{
    LastChoiceDialogueLine.Key = ChoiceSectionID;
    LastChoiceDialogueLine.Value = DialogueData;
    LastDialogueLine = {};
}

const FZWDialogueData& UZWDialogueChoiceSubsystem::GetDialogueLineToShowDuringChoice(const FGuid& ChoiceSectionID) const
{
    if (LastChoiceDialogueLine.Key == ChoiceSectionID)
    {
        LastChoiceDialogueLine.Value;
    }
    return LastDialogueLine;
}

void UZWDialogueChoiceSubsystem::SetPanelWidgetData(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& ChoiceData)
{
    CurrentChoiceData = ChoiceData;
    ChoiceStarted.Broadcast(ChoiceData);
    
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            if (UZWUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UZWUISubsystem>())
            {
                FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Panel.Prompt.DialogueChoice"));
                UISubsystem->OnGameplayTagSent.Broadcast(Tag);
            }
        }
    }
}

void UZWDialogueChoiceSubsystem::ReceiveDataFromQuestChoicePanelWidget(UZWDialogueChoiceChangeableObject* ChoiceData)
{
    UZWDialogueChoicePanelWidgetData* QuestChoicePanelWidgetData = Cast<UZWDialogueChoicePanelWidgetData>(ChoiceData);
    
    // Broadcast event to StateTree so it knows the choice has been confirmed and can close the panel
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (ULocalPlayer* LocalPlayer = GameInstance->GetFirstGamePlayer())
        {
            if (UZWUISubsystem* UISubsystem = LocalPlayer->GetSubsystem<UZWUISubsystem>())
            {
                FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName("UI.Dialogue.ChoiceSelected"));
                UISubsystem->OnGameplayTagSent.Broadcast(Tag);
            }
        }
    }
}

void UZWDialogueChoiceSubsystem::ResetChosenOptions()
{
    ChosenOptions.Empty();
}

void UZWDialogueChoiceSubsystem::ResetLastDialogueLines()
{
    LastDialogueLine = {};
    LastChoiceDialogueLine = {};
}
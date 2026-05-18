// Fill out your copyright notice in the Description page of Project Settings.

#include "UI/ZWMovieSceneDialogueWidget.h"
#include "Components/TextBlock.h"
#include "Engine/GameInstance.h"
#include "Blueprint/UserWidget.h"
#include "ZWMovieSceneDialogueSubsystem.h"

bool UZWMovieSceneDialogueWidget::Initialize()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        GameInstance->GetSubsystem<UZWMovieSceneDialogueSubsystem>()->RegisterDialogueHandler(this);
    }
    return Super::Initialize();
}

void UZWMovieSceneDialogueWidget::SetDialogueData(const FZWDialogueWidgetData& DialogueData)
{
    Speaker->SetText(DialogueData.Speaker);
    DialogueText->SetText(DialogueData.DialogueLine);

    if (DialogueData.Speaker.IsEmpty())
    {
        Speaker->SetVisibility(ESlateVisibility::Collapsed);
        Dots->SetVisibility(ESlateVisibility::Collapsed);
    }
    else
    {
        Speaker->SetVisibility(ESlateVisibility::Visible);
        Dots->SetVisibility(ESlateVisibility::Visible);
    }
}

FGuid UZWMovieSceneDialogueWidget::GetDialogueEventID() const
{
    return DialogueEventID;
}

uint32 UZWMovieSceneDialogueWidget::GetOrder() const
{
    // Should be the last one
    return 150u;
}

EZWStartDialogueResult UZWMovieSceneDialogueWidget::OnStartDialogueLine(const FZWDialogueData& DialogueData)
{
    DialogueEventID = DialogueData.EventID;
    FText SpeakerName = DialogueData.Speaker.IsEmpty() ? FText::FromName(DialogueData.SpeakerID) : DialogueData.Speaker;
    SetDialogueData(FZWDialogueWidgetData{ SpeakerName, DialogueData.DialogueLine }); 
    DialogueText->SetVisibility(ESlateVisibility::Visible);

    return EZWStartDialogueResult::Handled;
}

void UZWMovieSceneDialogueWidget::OnFinishDialogueLine(const FZWDialogueData& DialogueData)
{
    DialogueEventID = FGuid();
    SetDialogueData(FZWDialogueWidgetData{ FText::GetEmpty(), FText::GetEmpty() });
    DialogueText->SetVisibility(ESlateVisibility::Hidden);
}

void UZWMovieSceneDialogueWidget::NativeDestruct()
{
    if (UGameInstance* GameInstance = GetGameInstance())
    {
        GameInstance->GetSubsystem<UZWMovieSceneDialogueSubsystem>()->UnregisterDialogueHandler(this);
    }

    Super::NativeDestruct();
}
/*
Possibly needed when animation is included, but it's not handled atm
void UZWMovieSceneDialogueWidget::OnDialogueLineUpdated(const FZWDialogueData DialogueData)
{
    SetDialogueData(FZWDialogueWidgetData{DialogueData.SpeakerID, DialogueData.DialogueLine});
}
*/

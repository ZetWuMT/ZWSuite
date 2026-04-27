// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestChoicePanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Input/CommonUIInputTypes.h"
#include "QuestChoiceWidget.h"
#include "QuestChoiceData.h"

void UQuestChoicePanelWidget::SetupView(const TObjectPtr<UQuestChoicePanelWidgetData>& Data)
{
    ChoiceWidgetData = Cast<UQuestChoicePanelWidgetData>(Data);
    check(ChoiceWidgetData);

    int32 ChoiceIndex = 0;
    CurrentChoiceIndex = 0;

    for (UQuestChoiceData* ChoiceData : ChoiceWidgetData->MainChoices)
    {
        CreateChoice(ChoiceData, ChoiceIndex);
        ChoiceIndex++;
    }

    for (UQuestChoiceData* ChoiceData : ChoiceWidgetData->Choices)
    {
        CreateChoice(ChoiceData, ChoiceIndex);
        ChoiceIndex++;
    }

    SelectChoice(EChoiceSelection::Current);
}

void UQuestChoicePanelWidget::SelectAndConfirmChoiceAtIndex(int Index)
{
    if (Choices.Num() > Index)
    {
        Choices[CurrentChoiceIndex]->UnselectChoice();
        CurrentChoiceIndex = Index;
        Choices[CurrentChoiceIndex]->SelectChoice();

        ConfirmSelectedChoice();
    }
}

void UQuestChoicePanelWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (!NextChoiceActionHandle.IsValid() && !NextChoiceActionData.IsNull())
    {
        NextChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            NextChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UQuestChoicePanelWidget::SelectNextChoice)));
    }

    if (!PreviousChoiceActionHandle.IsValid() && !PreviousChoiceActionData.IsNull())
    {
        PreviousChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            PreviousChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UQuestChoicePanelWidget::SelectPreviousChoice)));
    }

    if (!ConfirmChoiceActionHandle.IsValid() && !ConfirmChoiceActionData.IsNull())
    {
        ConfirmChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            ConfirmChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UQuestChoicePanelWidget::ConfirmSelectedChoice)));
    }
}

void UQuestChoicePanelWidget::NativeOnDeactivated()
{
    bIsActive = false;
    Choices.Empty();
    ChoiceWidgetData = nullptr;
    ChoicesBox->ClearChildren();
    bAlreadySelectedChoice = false;
    
    Super::NativeOnDeactivated();
}

/*
FReply UQuestChoicePanelWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    UE_LOG(LogTemp, Log, TEXT("Wheel"));
    return FReply::Handled();
}

FReply UQuestChoicePanelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{    
    //return InKeyEvent.GetKey() == FKey("F") ? OnKeyUp(InGeometry, InKeyEvent).NativeReply : FReply::Unhandled();
    UE_LOG(LogTemp, Log, TEXT("BANG"));
    return FReply::Unhandled();
}*/

void UQuestChoicePanelWidget::ShowQuestChoicePanelWidget()
{
    if (!bIsActive)
    {
        //ActivateWidget();
        //this->SetFocus();
        bIsActive = true;
    }    
}

void UQuestChoicePanelWidget::HideQuestChoicePanelWidget()
{
    if (bIsActive)
    {
        DeactivateWidget();
        bIsActive = false;
        Choices.Empty();
        ChoiceWidgetData = nullptr;
        ChoicesBox->ClearChildren();
        bAlreadySelectedChoice = false;
    }
}

void UQuestChoicePanelWidget::SelectNextChoice()
{
    SelectChoice(EChoiceSelection::Next);
}

void UQuestChoicePanelWidget::SelectPreviousChoice()
{
    SelectChoice(EChoiceSelection::Previous);
}

void UQuestChoicePanelWidget::ConfirmSelectedChoice()
{
    if (bAlreadySelectedChoice)
    {
        return;
    }
    bAlreadySelectedChoice = true;
    //ChoiceWidgetData->ConfirmedChoice = Choices[CurrentChoiceIndex]->ChooseAndGetChoiceLabel();
    FString ConfirmedChoiceString = "Choice ";
    ConfirmedChoiceString.Append(FString::FromInt(CurrentChoiceIndex));
    FName ConfirmedChoiceName = FName(ConfirmedChoiceString);
    ChoiceWidgetData->ConfirmedChoice = ConfirmedChoiceName;
    ChoiceWidgetData->SetDirty(true);
}

UQuestChoicePanelWidgetData* UQuestChoicePanelWidget::GetChoiceWidgetData()
{
    if (ChoiceWidgetData != nullptr)
    {
        return ChoiceWidgetData;
    }
    
    return nullptr;
}

void UQuestChoicePanelWidget::CreateChoice(UQuestChoiceData* ChoiceData, int32 Index)
{
    if (ChoiceData->bSingleUse && ChoiceData->bWasChosen)
    {
        return;
    }

    UUserWidget* Widget = CreateWidget(this, ChoiceWidgetRef);
    ChoicesBox->AddChildToVerticalBox(Widget);

    UQuestChoiceWidget* ChoiceWidget = Cast<UQuestChoiceWidget>(Widget);
    ChoiceWidget->SetChoiceData(ChoiceData);

    Choices.Add(Index, ChoiceWidget);
}

void UQuestChoicePanelWidget::SelectChoice(EChoiceSelection ChoiceToSelect)
{
    switch (ChoiceToSelect)
    {
    case EChoiceSelection::Current:
        Choices[CurrentChoiceIndex]->SelectChoice();
        break;
    case EChoiceSelection::Next:
        if (CurrentChoiceIndex == Choices.Num() - 1)
        {
            return;
        }
        Choices[CurrentChoiceIndex]->UnselectChoice();
        CurrentChoiceIndex++;
        Choices[CurrentChoiceIndex]->SelectChoice();
        break;
    case EChoiceSelection::Previous:
        if (CurrentChoiceIndex == 0)
        {
            return;
        }
        Choices[CurrentChoiceIndex]->UnselectChoice();
        CurrentChoiceIndex--;
        Choices[CurrentChoiceIndex]->SelectChoice();
        break;
    }
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZWDialogueChoicePanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "Input/CommonUIInputTypes.h"
#include "UI/ZWDialogueChoiceWidget.h"
#include "ZWDialogueChoiceData.h"

void UZWDialogueChoicePanelWidget::SetupView(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& Data)
{
    ChoiceWidgetData = Cast<UZWDialogueChoicePanelWidgetData>(Data);
    check(ChoiceWidgetData);

    int32 ChoiceIndex = 0;
    CurrentChoiceIndex = 0;

    for (UZWDialogueChoiceData* ChoiceData : ChoiceWidgetData->MainChoices)
    {
        CreateChoice(ChoiceData, ChoiceIndex);
        ChoiceIndex++;
    }

    for (UZWDialogueChoiceData* ChoiceData : ChoiceWidgetData->Choices)
    {
        CreateChoice(ChoiceData, ChoiceIndex);
        ChoiceIndex++;
    }

    SelectChoice(EChoiceSelection::Current);
}

void UZWDialogueChoicePanelWidget::SelectAndConfirmChoiceAtIndex(int Index)
{
    if (Choices.Num() > Index)
    {
        Choices[CurrentChoiceIndex]->UnselectChoice();
        CurrentChoiceIndex = Index;
        Choices[CurrentChoiceIndex]->SelectChoice();

        ConfirmSelectedChoice();
    }
}

void UZWDialogueChoicePanelWidget::NativeOnActivated()
{
    Super::NativeOnActivated();

    if (!NextChoiceActionHandle.IsValid() && !NextChoiceActionData.IsNull())
    {
        NextChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            NextChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UZWDialogueChoicePanelWidget::SelectNextChoice)));
    }

    if (!PreviousChoiceActionHandle.IsValid() && !PreviousChoiceActionData.IsNull())
    {
        PreviousChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            PreviousChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UZWDialogueChoicePanelWidget::SelectPreviousChoice)));
    }

    if (!ConfirmChoiceActionHandle.IsValid() && !ConfirmChoiceActionData.IsNull())
    {
        ConfirmChoiceActionHandle = RegisterUIActionBinding(FBindUIActionArgs(
            ConfirmChoiceActionData,
            false,
            FSimpleDelegate::CreateUObject(this, &UZWDialogueChoicePanelWidget::ConfirmSelectedChoice)));
    }
}

void UZWDialogueChoicePanelWidget::NativeOnDeactivated()
{
    bIsActive = false;
    Choices.Empty();
    ChoiceWidgetData = nullptr;
    ChoicesBox->ClearChildren();
    bAlreadySelectedChoice = false;
    
    Super::NativeOnDeactivated();
}

/*
FReply UZWDialogueChoicePanelWidget::NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
    UE_LOG(LogTemp, Log, TEXT("Wheel"));
    return FReply::Handled();
}

FReply UZWDialogueChoicePanelWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{    
    //return InKeyEvent.GetKey() == FKey("F") ? OnKeyUp(InGeometry, InKeyEvent).NativeReply : FReply::Unhandled();
    UE_LOG(LogTemp, Log, TEXT("BANG"));
    return FReply::Unhandled();
}*/

void UZWDialogueChoicePanelWidget::ShowDialogueChoicePanelWidget()
{
    if (!bIsActive)
    {
        //ActivateWidget();
        //this->SetFocus();
        bIsActive = true;
    }    
}

void UZWDialogueChoicePanelWidget::HideDialogueChoicePanelWidget()
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

void UZWDialogueChoicePanelWidget::SelectNextChoice()
{
    SelectChoice(EChoiceSelection::Next);
}

void UZWDialogueChoicePanelWidget::SelectPreviousChoice()
{
    SelectChoice(EChoiceSelection::Previous);
}

void UZWDialogueChoicePanelWidget::ConfirmSelectedChoice()
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

UZWDialogueChoicePanelWidgetData* UZWDialogueChoicePanelWidget::GetChoiceWidgetData()
{
    if (ChoiceWidgetData != nullptr)
    {
        return ChoiceWidgetData;
    }
    
    return nullptr;
}

void UZWDialogueChoicePanelWidget::CreateChoice(UZWDialogueChoiceData* ChoiceData, int32 Index)
{
    if (ChoiceData->bSingleUse && ChoiceData->bWasChosen)
    {
        return;
    }

    UUserWidget* Widget = CreateWidget(this, ChoiceWidgetRef);
    ChoicesBox->AddChildToVerticalBox(Widget);

    UZWDialogueChoiceWidget* ChoiceWidget = Cast<UZWDialogueChoiceWidget>(Widget);
    ChoiceWidget->SetChoiceData(ChoiceData);

    Choices.Add(Index, ChoiceWidget);
}

void UZWDialogueChoicePanelWidget::SelectChoice(EChoiceSelection ChoiceToSelect)
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
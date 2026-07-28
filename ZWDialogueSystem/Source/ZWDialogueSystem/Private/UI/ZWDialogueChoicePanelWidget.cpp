// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZWDialogueChoicePanelWidget.h"
#include "Components/TextBlock.h"
#include "Components/VerticalBox.h"
#include "UI/ZWDialogueChoiceWidget.h"
#include "ZWDialogueChoiceData.h"
#include "ZWDialogueChoiceSubsystem.h"
#include "Engine/GameInstance.h"
#include "ZWUISubsystem.h"

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

    if (UGameInstance* GameInstance = GetGameInstance())
    {
        if (UZWDialogueChoiceSubsystem* Subsystem = GameInstance->GetSubsystem<UZWDialogueChoiceSubsystem>())
        {
            if (UZWDialogueChoicePanelWidgetData* Data = Subsystem->GetCurrentChoiceData())
            {
                SetupView(Data);
                Data->OnChange.AddUniqueDynamic(Subsystem, &UZWDialogueChoiceSubsystem::ReceiveDataFromQuestChoicePanelWidget);
            }
        }
    }

    if (ULocalPlayer* LP = GetOwningLocalPlayer())
    {
        if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
        {
            UISubsystem->OnGameplayTagSent.AddUObject(this, &UZWDialogueChoicePanelWidget::HandleInputTag);
        }
    }
}

void UZWDialogueChoicePanelWidget::NativeOnDeactivated()
{
    bIsActive = false;
    Choices.Empty();
    ChoiceWidgetData = nullptr;
    ChoicesBox->ClearChildren();
    bAlreadySelectedChoice = false;
    
    if (ULocalPlayer* LP = GetOwningLocalPlayer())
    {
        if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
        {
            UISubsystem->OnGameplayTagSent.RemoveAll(this);
        }
    }

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
    if (bAlreadySelectedChoice || !ChoiceWidgetData)
    {
        return;
    }
    bAlreadySelectedChoice = true;
    
    UZWDialogueChoiceData* SelectedChoiceData = nullptr;
    int32 MainChoicesCount = ChoiceWidgetData->MainChoices.Num();
    if (CurrentChoiceIndex < MainChoicesCount)
    {
        SelectedChoiceData = ChoiceWidgetData->MainChoices[CurrentChoiceIndex];
    }
    else if (CurrentChoiceIndex - MainChoicesCount < ChoiceWidgetData->Choices.Num())
    {
        SelectedChoiceData = ChoiceWidgetData->Choices[CurrentChoiceIndex - MainChoicesCount];
    }

    if (SelectedChoiceData && SelectedChoiceData->ChoiceLabel.IsValid())
    {
        ChoiceWidgetData->ConfirmedChoice = SelectedChoiceData->ChoiceLabel;
    }
    else
    {
        FString ConfirmedChoiceString = "Choice ";
        ConfirmedChoiceString.Append(FString::FromInt(CurrentChoiceIndex));
        ChoiceWidgetData->ConfirmedChoice = FName(*ConfirmedChoiceString);
    }

    ChoiceWidgetData->SetDirty(true);
}

void UZWDialogueChoicePanelWidget::HandleInputTag(FGameplayTag InputTag)
{
    if (InputTag.MatchesTagExact(ConfirmChoiceTag))
    {
        ConfirmSelectedChoice();
    }
    else if (InputTag.MatchesTagExact(NextChoiceTag))
    {
        SelectNextChoice();
    }
    else if (InputTag.MatchesTagExact(PreviousChoiceTag))
    {
        SelectPreviousChoice();
    }
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
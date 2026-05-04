// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/ZWDialogueChoiceWidget.h"

void UZWDialogueChoiceWidget::SelectChoice()
{
	ChoiceBoxText->SetColorAndOpacity(FLinearColor::Gray);
}

void UZWDialogueChoiceWidget::UnselectChoice()
{
	ChoiceBoxText->SetColorAndOpacity(FLinearColor::White);
}

void UZWDialogueChoiceWidget::SetChoiceData(UZWDialogueChoiceData* ChoiceData)
{
	//ChoiceBoxText->Text = ChoiceData->ChoiceText;
	ChoiceBoxText->SetText(ChoiceData->ChoiceText);
}

FName UZWDialogueChoiceWidget::ChooseAndGetChoiceLabel()
{
	return FName(ChoiceBoxText->GetText().ToString());
}
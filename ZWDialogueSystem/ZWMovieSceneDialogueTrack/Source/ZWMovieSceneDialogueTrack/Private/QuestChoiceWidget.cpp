// Fill out your copyright notice in the Description page of Project Settings.


#include "QuestChoiceWidget.h"

void UQuestChoiceWidget::SelectChoice()
{
	ChoiceBoxText->SetColorAndOpacity(FLinearColor::Gray);
}

void UQuestChoiceWidget::UnselectChoice()
{
	ChoiceBoxText->SetColorAndOpacity(FLinearColor::White);
}

void UQuestChoiceWidget::SetChoiceData(UQuestChoiceData* ChoiceData)
{
	//ChoiceBoxText->Text = ChoiceData->ChoiceText;
	ChoiceBoxText->SetText(ChoiceData->ChoiceText);
}

FName UQuestChoiceWidget::ChooseAndGetChoiceLabel()
{
	return FName(ChoiceBoxText->GetText().ToString());
}
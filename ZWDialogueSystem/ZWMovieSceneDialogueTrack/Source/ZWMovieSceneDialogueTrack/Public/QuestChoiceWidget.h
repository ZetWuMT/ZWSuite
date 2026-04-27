// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestChoiceData.h"
#include "Components/TextBlock.h"
#include "CommonActivatableWidget.h"
#include "QuestChoiceWidget.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UQuestChoiceWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* ChoiceBoxText = nullptr;

	void SelectChoice();
	void UnselectChoice();

	void SetChoiceData(UQuestChoiceData* ChoiceData);
	FName ChooseAndGetChoiceLabel();
	
};

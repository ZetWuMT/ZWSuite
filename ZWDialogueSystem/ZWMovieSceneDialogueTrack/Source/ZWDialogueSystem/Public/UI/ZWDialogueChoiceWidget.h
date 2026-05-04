// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "ZWDialogueChoiceData.h"
#include "Components/TextBlock.h"
#include "CommonActivatableWidget.h"
#include "ZWDialogueChoiceWidget.generated.h"

/**
 * 
 */
UCLASS()
class ZWDIALOGUESYSTEM_API UZWDialogueChoiceWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (BindWidget))
	UTextBlock* ChoiceBoxText = nullptr;

	void SelectChoice();
	void UnselectChoice();

	void SetChoiceData(UZWDialogueChoiceData* ChoiceData);
	FName ChooseAndGetChoiceLabel();
	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueChoiceData.h"
#include "ZWDialogueChoiceChangeableObject.h"
#include "ZWDialogueChoicePanelWidgetData.generated.h"

/**
 * 
 */
UCLASS()
class ZWDIALOGUESYSTEM_API UZWDialogueChoicePanelWidgetData : public UZWDialogueChoiceChangeableObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TArray<TObjectPtr<UZWDialogueChoiceData>> MainChoices;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TArray<TObjectPtr<UZWDialogueChoiceData>> Choices;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FName ConfirmedChoice;
};

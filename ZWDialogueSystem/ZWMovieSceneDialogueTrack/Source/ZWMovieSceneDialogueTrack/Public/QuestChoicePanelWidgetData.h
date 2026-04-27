// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestChoiceData.h"
#include "QuestChoiceChangeableObject.h"
#include "QuestChoicePanelWidgetData.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UQuestChoicePanelWidgetData : public UQuestChoiceChangeableObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TArray<TObjectPtr<UQuestChoiceData>> MainChoices;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    TArray<TObjectPtr<UQuestChoiceData>> Choices;

    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
    FName ConfirmedChoice;
};

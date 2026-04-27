// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Delegates/Delegate.h"
#include "QuestChoiceChangeableObject.h"
#include "QuestChoiceData.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UQuestChoiceData : public UQuestChoiceChangeableObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FText ChoiceText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName ChoiceLabel;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FName IconPreset;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bWasChosen = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bSingleUse = false;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool bMainChoice = false;
};

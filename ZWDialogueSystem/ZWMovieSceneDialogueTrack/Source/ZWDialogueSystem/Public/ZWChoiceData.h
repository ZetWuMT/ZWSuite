// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "ZWChoiceData.generated.h"

/**
 * 
 */
USTRUCT()
struct ZWDIALOGUESYSTEM_API FZWDialogueChoice
{
    GENERATED_BODY()

    FZWDialogueChoice() = default;

    FZWDialogueChoice(FGuid Guid) : Guid(Guid)
    {
    }

    UPROPERTY(VisibleAnywhere, Category = "Choice", Meta = (NoResetToDefault))
    FGuid Guid;

    UPROPERTY(EditAnywhere, Category = "Choice")
    FText ChoiceText;

    UPROPERTY(EditAnywhere, Category = "Choice")
    FName SocketName;

    UPROPERTY(EditAnywhere, Category = "Choice")
    bool bMainChoice = false;

    UPROPERTY(EditAnywhere, Category = "Choice")
    bool bSingleUse = false;

    /*Optional Quality of Life elements
    UPROPERTY(EditAnywhere, Category = "Choice", meta = (GetOptions = "GetChoiceIconStrings"))
    FName IconPreset = ChoicePreset_Automatic;

    UPROPERTY(EditAnywhere, Category = "Choice", meta = (...))
    FString GetConditionsString(const UFlowNode* Node) const;*/

};

USTRUCT()
struct ZWDIALOGUESYSTEM_API FZWChoiceData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, Category = "Choices", /*EditFixedSize,*/ meta = (TitleProperty = "SocketName"))
    TArray<FZWDialogueChoice> Choices;
};
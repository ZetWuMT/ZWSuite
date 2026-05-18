// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZWDialogueLineHandler.generated.h"

UINTERFACE(Blueprintable)
class ZWDIALOGUESYSTEM_API UZWDialogueLineHandler : public UInterface
{
    GENERATED_BODY()
};

class ZWDIALOGUESYSTEM_API IZWDialogueLineHandler
{
    GENERATED_BODY()

public:
    virtual uint32 GetOrder() const = 0;

    virtual EZWStartDialogueResult OnStartDialogueLine(const FZWDialogueData& DialogueData) = 0;
    virtual void OnFinishDialogueLine(const FZWDialogueData& DialogueData) = 0;
    virtual void OnDialogueLineUpdated(const FZWDialogueData& DialogueData) {}
};
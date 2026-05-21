// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueLineHandler.h"
#include "ZWMovieSceneDialogueSubsystem.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h"
#include "ZWMovieSceneDialogueWidget.generated.h"

//class UTextBlock;

USTRUCT(BlueprintType)
struct FZWDialogueWidgetData
{
    GENERATED_BODY()
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "SpeakerID")
    FText Speaker;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dialogue Line")
    FText DialogueLine;
};

/**
 * 
 */
UCLASS(Abstract)
class ZWDIALOGUESYSTEM_API UZWMovieSceneDialogueWidget : public UUserWidget, public IZWDialogueLineHandler
{
	GENERATED_BODY()
	
public:
    virtual bool Initialize() override;

    void SetDialogueData(const FZWDialogueWidgetData& DialogueData);
    FGuid GetDialogueEventID() const;

    // This is currently not needed, but it will be when more systems are introduced
    virtual uint32 GetOrder() const override;

    virtual EZWStartDialogueResult OnStartDialogueLine(const FZWDialogueData& DialogueData) override;
    virtual void OnFinishDialogueLine(const FZWDialogueData& DialogueData) override;
    /*
    Possibly needed when animation is included, but it's not handled atm
    virtual void OnDialogueLineUpdated(const FZWDialogueData& DialogueData) override;
    */

private:
    virtual void NativeDestruct() override;

    FGuid DialogueEventID;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
    //UTextBlock* SpeakerID;
    TObjectPtr<UTextBlock> Speaker;

	UPROPERTY(EditAnywhere, meta = (BindWidget))
    //UTextBlock* DialogueText;
    TObjectPtr<UTextBlock> DialogueText;

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    //UTextBlock* Dots;
    TObjectPtr<UTextBlock> Dots;
};

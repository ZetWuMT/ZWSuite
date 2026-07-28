// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Containers/Map.h"
#include "CoreMinimal.h"
#include "ZWDialogueData.h"
#include "ZWDialogueLineHandler.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZWMovieSceneDialogueSubsystem.h"
#include "UI/ZWDialogueChoicePanelWidgetData.h"
#include "ZWDialogueChoiceSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnChoiceStartedDelegate, const TObjectPtr<UZWDialogueChoicePanelWidgetData>&);

/**
 * 
 */
UCLASS()
class ZWDIALOGUESYSTEM_API UZWDialogueChoiceSubsystem : public UGameInstanceSubsystem, public IZWDialogueLineHandler
{
	GENERATED_BODY()

public:   
    FOnChoiceStartedDelegate ChoiceStarted;

    // Begin UGameInstanceSubsystem
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    //virtual void Deinitialize() override;
    // End UGameInstanceSubsystem

    //Begin IZWDialogueLineHandler    
    virtual uint32 GetOrder() const override;
    virtual EZWStartDialogueResult OnStartDialogueLine(const FZWDialogueData& DialogueData) override;
    virtual void OnFinishDialogueLine(const FZWDialogueData& DialogueData) override;
    //End IZWDialogueLineHandler

    void ChooseOption(const FGuid& NodeGuid, FName Choice);
    bool WasOptionChosen(const FGuid& NodeGuid, FName Choice);

    void OnShowChoiceDialogueLine(const FGuid& ChoiceSectionID, const FZWDialogueData& DialogueData);
    const FZWDialogueData& GetDialogueLineToShowDuringChoice(const FGuid& ChoiceSectionID) const;

    void SetPanelWidgetData(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& ChoiceData);
    
    UFUNCTION(BlueprintPure, Category = "Dialogue|Choices")
    UZWDialogueChoicePanelWidgetData* GetCurrentChoiceData() const { return CurrentChoiceData; }

	UFUNCTION()
    void ReceiveDataFromQuestChoicePanelWidget(UZWDialogueChoiceChangeableObject* ChoiceData);
	
private:
    UPROPERTY()
    TObjectPtr<UZWDialogueChoicePanelWidgetData> CurrentChoiceData;
    void ResetChosenOptions();
    void ResetLastDialogueLines();

    TMultiMap<FGuid, FName> ChosenOptions;
    FZWDialogueData LastDialogueLine;
    TPair<FGuid, FZWDialogueData> LastChoiceDialogueLine;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "QuestChoiceData.h"
#include "QuestChoiceWidget.h"
#include "QuestChoicePanelWidgetData.h"
#include "Components/VerticalBox.h"
#include "CommonActivatableWidget.h"
#include "QuestChoicePanelWidget.generated.h"

class UQuestChoiceData;
class UProgressBar;
class UVerticalBox;

UENUM()
enum class EChoiceSelection : uint8
{
    Current,
    Next,
    Previous
};

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UQuestChoicePanelWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    void SetupView(const TObjectPtr<UQuestChoicePanelWidgetData>& ChoiceData);

    UFUNCTION(BlueprintCallable, Category = DialogueUI)
    void SelectAndConfirmChoiceAtIndex(int Index);

    const TMap<int, UQuestChoiceWidget*>& GetChoices() const
    {
        return Choices;
    }

    void ShowQuestChoicePanelWidget();
    void HideQuestChoicePanelWidget();

    UQuestChoicePanelWidgetData* GetChoiceWidgetData();

protected:
    virtual void NativeOnActivated() override;
    virtual void NativeOnDeactivated() override;
    /*
    virtual FReply NativeOnMouseWheel(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent) override;
    virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;*/

    bool bIsActive = false;

    UFUNCTION(BlueprintCallable)
    void SelectNextChoice();

    UFUNCTION(BlueprintCallable)
    void SelectPreviousChoice();

    UFUNCTION(BlueprintCallable)
    void ConfirmSelectedChoice();

    UPROPERTY(EditAnywhere, meta = (BindWidget))
    UVerticalBox* ChoicesBox = nullptr;

    UPROPERTY(EditAnywhere)
    TSubclassOf<UQuestChoiceWidget> ChoiceWidgetRef;

    UPROPERTY(EditDefaultsOnly, Category = "Input Actions Data")
    FDataTableRowHandle NextChoiceActionData;

    UPROPERTY(EditDefaultsOnly, Category = "Input Actions Data")
    FDataTableRowHandle PreviousChoiceActionData;

    UPROPERTY(EditDefaultsOnly, Category = "Input Actions Data")
    FDataTableRowHandle ConfirmChoiceActionData;

    FUIActionBindingHandle NextChoiceActionHandle;
    FUIActionBindingHandle PreviousChoiceActionHandle;
    FUIActionBindingHandle ConfirmChoiceActionHandle;

    //virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

    int32 CurrentChoiceIndex = 0;   

private:
    //virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
    void CreateChoice(UQuestChoiceData* ChoiceData, int32 Index);
    void SelectChoice(EChoiceSelection ChoiceToSelect);

    UPROPERTY()
    UQuestChoicePanelWidgetData* ChoiceWidgetData = nullptr;
    TMap<int, UQuestChoiceWidget*> Choices;
    bool bAlreadySelectedChoice = false;
};

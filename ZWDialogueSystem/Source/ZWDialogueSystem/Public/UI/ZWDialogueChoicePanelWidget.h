// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueChoiceData.h"
#include "ZWDialogueChoiceWidget.h"
#include "ZWDialogueChoicePanelWidgetData.h"
#include "Components/VerticalBox.h"
#include "ZWUIPanel.h"
#include "ZWDialogueChoicePanelWidget.generated.h"

class UZWDialogueChoiceData;
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
class ZWDIALOGUESYSTEM_API UZWDialogueChoicePanelWidget : public UZWUIPanel
{
    GENERATED_BODY()

public:
    void SetupView(const TObjectPtr<UZWDialogueChoicePanelWidgetData>& ChoiceData);

    UFUNCTION(BlueprintCallable, Category = DialogueUI)
    void SelectAndConfirmChoiceAtIndex(int Index);

    const TMap<int, UZWDialogueChoiceWidget*>& GetChoices() const
    {
        return Choices;
    }

    void ShowDialogueChoicePanelWidget();
    void HideDialogueChoicePanelWidget();

    UZWDialogueChoicePanelWidgetData* GetChoiceWidgetData();

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
    TSubclassOf<UZWDialogueChoiceWidget> ChoiceWidgetRef;

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
    void CreateChoice(UZWDialogueChoiceData* ChoiceData, int32 Index);
    void SelectChoice(EChoiceSelection ChoiceToSelect);

    UPROPERTY()
    UZWDialogueChoicePanelWidgetData* ChoiceWidgetData = nullptr;
    TMap<int, UZWDialogueChoiceWidget*> Choices;
    bool bAlreadySelectedChoice = false;
};

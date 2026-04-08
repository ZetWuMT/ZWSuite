// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZWUIPanel.h"
#include "ZWUIPanelDatabase.h"
#include "Engine/DeveloperSettings.h"
#include "ZWUISettings.generated.h"

class UZWInputConfig;
class UZWUIRootLayout;
class UZWUIPlayerHUBWidget;
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW UI Core"))
class ZWUICORE_API UZWUISettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UZWUISettings();
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Features", DisplayName=" Main Root Layout Class")
	TSubclassOf<UUserWidget> MainRootLayoutClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Features", DisplayName=" Main HUD Class")
	TSoftClassPtr<UZWUIPanel> MainHUDClass;
	/**
	 * The class of the Main Player HUB widget.
	 * It will show all the fullscreen widgets (Inventory, Player Progression, Pause Menu etc.).
	 * NOTE: It's NOT a HUD widget! The HUD widget is a separate widget for all in-game widgets. It is specified in GameMode.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "UI Features", DisplayName=" Main HUB Class")
	TSoftClassPtr<UZWUIPlayerHUBWidget> MainHUBClass;
	
	UPROPERTY(Config, EditAnywhere, Category = "UI Features", DisplayName=" Main HUB Tag", meta = (EditCondition = "bIsUIStateExternallyManaged", EditConditionHides))
	FGameplayTag MainHUBTag;
	
	UPROPERTY(Config, EditAnywhere, Category= "UI Registry", meta=(ForceInlineRow))
	TSoftObjectPtr<UZWUIPanelDatabase> PanelRegistry;
	
	UPROPERTY(Config, EditAnywhere, Category= "UI Registry", meta=(ForceInlineRow))
	TMap<FGameplayTag, TSoftClassPtr<UUserWidget>> PromptPanels;
	
	UPROPERTY(Config, EditAnywhere, Category= "Input", meta=(ForceInlineRow))
	TSoftObjectPtr<UZWInputConfig> InputConfig;
	
	/**
    * If TRUE, panels do NOT close themselves after pressing the return/exit action.
    * The system assumes that an external manager (e.g., ZWUIStateTree) will receive the Broadcast Tag
    * and trigger the panel removal itself (e.g., via the 'Close Panel' Task).
    */
	UPROPERTY(Config, EditAnywhere, Category = "UI Architecture")
	bool bIsUIStateExternallyManaged = false;
	
	/**
	 * A tag that is sent to an external system to inform it that the window should be closed.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "UI Architecture", meta = (EditCondition = "bIsUIStateExternallyManaged", EditConditionHides))
	FGameplayTag ExternalCloseTag;
	
#if WITH_EDITORONLY_DATA
	virtual FName GetCategoryName() const override { return FName("ZW"); }
	virtual FText GetSectionText() const override { return INVTEXT("ZW UI Core Settings"); }
#endif
	
private:
	UPROPERTY()
	TMap<FGameplayTag, TSoftClassPtr<UZWUIPanel>> MenuPanels;
};

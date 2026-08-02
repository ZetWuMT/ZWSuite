// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "Engine/StreamableManager.h"
#include "ZWUIPanel.h"
#include "ZWUIPlayerHUBWidget.generated.h"

class UCommonTabListWidgetBase;
class UZWUITabConfig;
class UCommonBoundActionBar;
enum class EZWWidgetLayer : uint8;
class UOverlay;
class UCommonActivatableWidgetStack;
class UCommonActivatableWidgetSwitcher;

/**
 * The main container for all UI panels. 
 * Listens to the UI Subsystem and dynamically loads/switches panels using a Switcher.
 */
UCLASS(Abstract, Blueprintable)
class ZWUICORE_API UZWUIPlayerHUBWidget : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UZWUIPlayerHUBWidget(const FObjectInitializer& ObjectInitializer);
	
	UFUNCTION(BlueprintCallable, Category = "UI|Visuals")
	void SetMenuBackgroundVisible(bool bVisible);
	
	void OpenTabInSwitcher(UZWUIPanel* Panel);
	
	void OnTabClosed();
	
	UFUNCTION(BlueprintPure, Category = "ZW|UI")
	UZWUIPanel* GetInstancedTab(FGameplayTag TabTag) const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual void NativeOnDeactivated() override;
	
	UFUNCTION()
	void HandleTabSelected(FName TabId);
	
	UPROPERTY(meta = (BindWidget))
	UWidget* MenuBackground;

	/** The switcher that holds menu panels (Inventory, Journal, etc.). Requires background. */
	UPROPERTY(meta = (BindWidget))
	UCommonActivatableWidgetSwitcher* MenuPanelsSwitcher;
	
	UPROPERTY(meta = (BindWidget))
	UOverlay* MenuPanelsOverlay;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	UCommonBoundActionBar* FooterActionBar;
	
	// Optional Data Asset. If the BP author hooks it up, the HUB will generate the buttons.
	UPROPERTY(EditDefaultsOnly, Category = "ZW|Tabs")
	TObjectPtr<UZWUITabConfig> TabConfiguration;

	// MAGIC OF OPTIONALITY: The tab bar. If it is not in the BP, this will be nullptr.
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTabListWidgetBase> MenuTabList;

private:
	/** Callback fired when FStreamableManager finishes loading the class from disk */
	void OnWidgetClassLoaded(FGameplayTag TabTag, TSoftClassPtr<UZWUIPanel> SoftWidgetClass);
	
	/** Cache of already instantiated widgets to avoid recreating them */
	UPROPERTY()
	TMap<FGameplayTag, UZWUIPanel*> InstancedTabs;

	/** Keeps track of active load requests to prevent duplicate async load calls */
	TMap<FGameplayTag, TSharedPtr<FStreamableHandle>> ActiveLoadHandles;
};
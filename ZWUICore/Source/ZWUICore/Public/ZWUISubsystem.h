// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "ZWUIPanelDatabase.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ZWUISubsystem.generated.h"

class UCommonActivatableWidgetStack;
struct FStreamableHandle;
class UZWUIRootLayout;
class UOverlay;
class UZWUIPlayerHUBWidget;
enum class EZWWidgetLayer : uint8;
class UZWUIPanel;
class UCommonUIActionRouterBase;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUITagSentDelegate, FGameplayTag);

USTRUCT()
struct FZWActivePanelContext
{
	GENERATED_BODY()

	UPROPERTY()
	UZWUIPanel* Panel = nullptr;

	UPROPERTY()
	FGameplayTag PanelTag;

	// Przeciążenie operatora pozwala nam wyszukiwać w tablicy po samym wskaźniku
	bool operator==(const UZWUIPanel* OtherPanel) const
	{
		return Panel == OtherPanel;
	}
};

/**
 * 
 */
UCLASS()
class ZWUICORE_API UZWUISubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()
	
public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;	
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~ End USubsystem Interface
	
	/** Registers a specific widget class to a tag (e.g., "UI.Tab.Inventory") */
	UFUNCTION(BlueprintCallable, Category = "ZW|UI")
	void RegisterPanelData(const FZWUIPanelData& PanelData);

	/** Call this from your Enhanced Input actions to request opening a panel */
	UFUNCTION(BlueprintCallable, Category = "ZW|UI|Input")
	void RequestPanelWidget(FGameplayTag PanelTag);
	
	// Pobiera z pamięci lub tworzy (jeśli nie istnieje) panel dla danego Taga.
	UFUNCTION(BlueprintCallable, Category = "ZW|UI")
	UZWUIPanel* GetOrCreateInstancedPanel(FGameplayTag PanelTag);
	
	UFUNCTION(BlueprintCallable, Category = "ZW|UI")
	void ClosePanelWidget(FGameplayTag PanelTag);
	
	UFUNCTION(BlueprintPure, Category = "ZW|UI")
	UZWUIRootLayout* GetRootLayout() const { return RootLayout; }

	void RegisterPanel(UZWUIPanel* Panel);
	void UnregisterPanel(UZWUIPanel* Panel);
	
	bool IsPanelRegisteredByTag(FGameplayTag);
	
	//UPROPERTY(BlueprintAssignable, Category = "ZW|UI|Events")
	FOnUITagSentDelegate OnGameplayTagSent;
	
	/** Returns the soft class pointer for a given tag */
	UFUNCTION(BlueprintPure, Category = "ZW|UI")
	TSoftClassPtr<UZWUIPanel> GetPanelClass(FGameplayTag PanelTag) const;
	
protected:
	void RefreshInputConfig();

private:
	UCommonUIActionRouterBase* GetActionRouter() const;
	
	bool AddPanelToLayer(FGameplayTag PanelTag, TSubclassOf<UZWUIPanel> PanelClass);
	
	UFUNCTION()
	void OnPanelWidgetClosed();
	
	void OnPanelClassLoaded(FGameplayTag PanelTag, TSoftClassPtr<UZWUIPanel> SoftPanelClass);

	/** Cache of already instantiated widgets to avoid recreating them */
	UPROPERTY()
	TMap<FGameplayTag, UZWUIPanel*> InstancedPanels;
	
	/** Keeps track of active load requests to prevent duplicate async load calls */
	TMap<FGameplayTag, TSharedPtr<FStreamableHandle>> ActiveLoadHandles;
	
	/** Map storing the registry of GameplayTag to Widget Class */
	UPROPERTY()
	TMap<FGameplayTag, FZWUIPanelData> RegisteredPanels;

	UPROPERTY()
	TArray<FZWActivePanelContext> ActivePanels;
	
	UPROPERTY()
	UZWUIRootLayout* RootLayout;
	
	UPROPERTY()
	UZWUIPanel* HUD;
	
	UPROPERTY()
	UZWUIPlayerHUBWidget* PlayerHUB;
	
	UPROPERTY()
	UCommonActivatableWidgetStack* PromptStack;
	
	UPROPERTY()
	APlayerController* PlayerController;
};

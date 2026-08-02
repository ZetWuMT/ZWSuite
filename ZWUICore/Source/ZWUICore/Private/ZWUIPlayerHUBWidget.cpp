// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWUIPlayerHUBWidget.h"
#include "CommonActivatableWidgetSwitcher.h"
#include "CommonButtonBase.h"
#include "CommonTabListWidgetBase.h"
#include "ZWUISettings.h"
#include "ZWUISubsystem.h"
#include "ZWUITabConfig.h"
#include "ZWUITabListButton.h"
#include "Components/Overlay.h"
#include "Engine/GameInstance.h"
#include "Engine/AssetManager.h"
#include "Input/CommonUIInputTypes.h"

UZWUIPlayerHUBWidget::UZWUIPlayerHUBWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// The HUB itself doesn't need to be modal, it just manages the modal panels inside it.
	//bIsModal = false;
}

void UZWUIPlayerHUBWidget::SetMenuBackgroundVisible(bool bVisible)
{
	if (MenuBackground)
	{
		MenuBackground->SetVisibility(bVisible ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UZWUIPlayerHUBWidget::OpenTabInSwitcher(UZWUIPanel* Panel)
{
	if (!Panel || !MenuPanelsSwitcher) return;
	
	if (!MenuPanelsSwitcher->HasChild(Panel))
	{
		MenuPanelsSwitcher->AddChild(Panel);	
	}
	
	if (MenuTabList)
	{
		FName TabID = Panel->BoundPanelTag.GetTagName();
		
		// We do not check "GetSelectedTabId() != TabID", because even if the tab
		// is selected, the panel may be deactivated. Common UI is smart
		// and will fire ActivateWidget() by itself (which in turn correctly calls your RegisterPanel!).
		MenuTabList->SelectTabByID(TabID);		
	}
	else
	{
		if (MenuPanelsSwitcher->GetActiveWidget() != Panel)
		{
			MenuPanelsSwitcher->SetActiveWidget(Panel);
		}
		
		if (!Panel->IsActivated())
		{
			Panel->ActivateWidget();	
		}
	}

	// 4. Common logic for updating the background and events (for both paths)
	SetMenuBackgroundVisible(Panel->bRequiresBackground);
	
	// Hook into the panel close (we use Remove to prevent double binding!)
	Panel->OnDeactivated().RemoveAll(this);
	Panel->OnDeactivated().AddUObject(this, &UZWUIPlayerHUBWidget::OnTabClosed);
}

void UZWUIPlayerHUBWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Get our Boss (Subsystem)
	UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>();

	if (MenuTabList && MenuPanelsSwitcher && TabConfiguration && Subsystem)
	{
		MenuTabList->SetLinkedSwitcher(MenuPanelsSwitcher);
		
		MenuTabList->OnTabSelected.AddUniqueDynamic(this, &UZWUIPlayerHUBWidget::HandleTabSelected);

		for (const FZWUITabDefinition& TabDef : TabConfiguration->Tabs)
		{
			FName TabID = TabDef.TabTag.GetTagName();
			if (MenuTabList->GetTabButtonBaseByID(TabID)) continue;

			// THE HUB ASKS THE SUBSYSTEM FOR THE PANEL (No freelancing!)
			if (UZWUIPanel* Panel = Subsystem->GetOrCreateInstancedPanel(TabDef.TabTag))
			{
				// Since the Subsystem gave us a proper pointer, the rest is a formality
				if (!MenuPanelsSwitcher->HasChild(Panel))
				{
					MenuPanelsSwitcher->AddChild(Panel);
				}

				bool bRegistered = MenuTabList->RegisterTab(TabID, TabDef.TabButtonClass, Panel);
				if (bRegistered)
				{
					if (UCommonButtonBase* CreatedButton = MenuTabList->GetTabButtonBaseByID(TabID))
					{
						if (UZWUITabListButton* MyTabBtn = Cast<UZWUITabListButton>(CreatedButton))
						{
							MyTabBtn->SetTabData(TabDef.TabDisplayName, TabDef.TabIcon);
						}
					}
				}
				
				// We use AddUniqueDynamic/AddUObject to hook up
				Panel->OnDeactivated().RemoveAll(this);
				Panel->OnDeactivated().AddUObject(this, &UZWUIPlayerHUBWidget::OnTabClosed);
			}
		}
	}
}

void UZWUIPlayerHUBWidget::NativeDestruct()
{
	// Cancel any pending asynchronous loads to prevent crashes or memory leaks
	for (auto& KVP : ActiveLoadHandles)
	{
		if (KVP.Value.IsValid() && KVP.Value->IsActive())
		{
			KVP.Value->CancelHandle();
		}
	}
	ActiveLoadHandles.Empty();

	Super::NativeDestruct();
}

void UZWUIPlayerHUBWidget::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();

	// 1. CHECK WHO IS IN CHARGE:
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// SILENCE! The HUB is closed by the State Tree (e.g. by the 'Close Panel' Task)
		// or when switching tabs. The State Tree already knows everything.
		return; 
	}

	// 2. FALLBACK FOR ZWUICORE ALONE:
	// If the plugin works standalone, we use the old way to inform e.g. the game about closing the menu
	if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
	{
		FGameplayTag BackTag = FGameplayTag::RequestGameplayTag(FName("UI.State.Back"));
		Subsystem->OnGameplayTagSent.Broadcast(BackTag);
	}
}

void UZWUIPlayerHUBWidget::HandleTabSelected(FName TabId)
{
	// 1. Guard against missing configuration and clicking the same tab
	if (!TabConfiguration || (MenuPanelsSwitcher && MenuPanelsSwitcher->IsCurrentlySwitching())) return;

	FGameplayTag StateTagToSend;

	// 2. Look up the tab definition in our Data Asset
	for (const FZWUITabDefinition& TabDef : TabConfiguration->Tabs)
	{
		// Common UI returns the TabId as FName, so we compare
		if (TabDef.TabTag.GetTagName() == TabId)
		{
			StateTagToSend = TabDef.StateTag;
			break;
		}
	}
	
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// SILENCE! The HUB is closed by the State Tree (e.g. by the 'Close Panel' Task)
		// or when switching tabs. The State Tree already knows everything.
		return; 
	}

	// 3. If we found a State Tag, send it to the state machine!
	if (StateTagToSend.IsValid())
	{
		if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			// Use your universal transmitter
			Subsystem->OnGameplayTagSent.Broadcast(StateTagToSend);
			
			UE_LOG(LogTemp, Log, TEXT("HUB: Tab changed with mouse. Sending to State Tree: %s"), *StateTagToSend.ToString());
		}
	}
}

void UZWUIPlayerHUBWidget::OnWidgetClassLoaded(FGameplayTag TabTag, TSoftClassPtr<UZWUIPanel> SoftWidgetClass)
{
	ActiveLoadHandles.Remove(TabTag);

	TSubclassOf<UZWUIPanel> WidgetClass = SoftWidgetClass.Get();
	if (!WidgetClass) return;

	UZWUIPanel* NewTab = CreateWidget<UZWUIPanel>(this, WidgetClass);
	if (NewTab)
	{
		NewTab->PanelIdentityTag = TabTag;		
		InstancedTabs.Add(TabTag, NewTab);
		
		MenuPanelsSwitcher->AddChild(NewTab);		
		MenuPanelsSwitcher->SetActiveWidget(NewTab);
		if (!NewTab->IsActivated())
		{
			NewTab->ActivateWidget();	
		}
		SetMenuBackgroundVisible(NewTab->bRequiresBackground);
		
		NewTab->OnDeactivated().AddUObject(this, &UZWUIPlayerHUBWidget::OnTabClosed);
	}
}

void UZWUIPlayerHUBWidget::OnTabClosed()
{
	//if (!IsActivated()) return;
	
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		return; 
	}
	
	if (MenuPanelsSwitcher && MenuPanelsSwitcher->IsCurrentlySwitching()) return;
	
	UZWUIPanel* ActivePanel = Cast<UZWUIPanel>(MenuPanelsSwitcher->GetActiveWidget());
	
	// If the Switcher is empty OR its widget is disabled -> turn off the background
	if (!ActivePanel || !ActivePanel->IsActivated())
	{
		SetMenuBackgroundVisible(false);
		DeactivateWidget();
	}
	else
	{
		// If for some reason another widget became active, make sure
		// whether it requires the background (we read our flag)
		SetMenuBackgroundVisible(ActivePanel->bRequiresBackground);
	}	
}

UZWUIPanel* UZWUIPlayerHUBWidget::GetInstancedTab(FGameplayTag TabTag) const
{
	if (UZWUIPanel* const* FoundTab = InstancedTabs.Find(TabTag))
	{
		return *FoundTab;
	}
	return nullptr;
}

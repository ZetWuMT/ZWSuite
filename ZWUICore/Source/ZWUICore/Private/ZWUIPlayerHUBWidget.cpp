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
		
		// Nie sprawdzamy "GetSelectedTabId() != TabID", bo nawet jeśli zakładka 
		// jest zaznaczona, panel może być deaktywowany. Common UI jest mądre 
		// i samo odpali ActivateWidget() (co z kolei poprawnie wywoła Twój RegisterPanel!).
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

	// 4. Wspólna logika aktualizacji tła i eventów (dla obu ścieżek)
	SetMenuBackgroundVisible(Panel->bRequiresBackground);
	
	// Podpinamy się pod zamknięcie panelu (używamy Remove, żeby zapobiec podwójnemu bindowaniu!)
	Panel->OnDeactivated().RemoveAll(this);
	Panel->OnDeactivated().AddUObject(this, &UZWUIPlayerHUBWidget::OnTabClosed);
}

void UZWUIPlayerHUBWidget::NativeConstruct()
{
	Super::NativeConstruct();
	
	// Pobieramy naszego Szefa (Subsystem)
	UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>();

	if (MenuTabList && MenuPanelsSwitcher && TabConfiguration && Subsystem)
	{
		MenuTabList->SetLinkedSwitcher(MenuPanelsSwitcher);
		
		MenuTabList->OnTabSelected.AddUniqueDynamic(this, &UZWUIPlayerHUBWidget::HandleTabSelected);

		for (const FZWUITabDefinition& TabDef : TabConfiguration->Tabs)
		{
			FName TabID = TabDef.TabTag.GetTagName();
			if (MenuTabList->GetTabButtonBaseByID(TabID)) continue;

			// HUB PROSI SUBSYSTEM O PANEL (Zero samowolki!)
			if (UZWUIPanel* Panel = Subsystem->GetOrCreateInstancedPanel(TabDef.TabTag))
			{
				// Z racji tego, że Subsystem oddał nam prawilny wskaźnik, reszta to formalność
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
				
				// Używamy AddUniqueDynamic/AddUObject, żeby się podpiąć
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

	// 1. SPRAWDZAMY KTO RZĄDZI:
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// CISZA! HUB jest zamykany przez State Tree (np. przez Task 'Close Panel') 
		// lub podczas zmiany zakładek. State Tree już o wszystkim wie.
		return; 
	}

	// 2. FALLBACK DLA SAMEGO ZWUICORE:
	// Jeśli plugin działa samodzielnie, używamy starego sposobu do poinformowania np. gry o zamknięciu menu
	if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
	{
		FGameplayTag BackTag = FGameplayTag::RequestGameplayTag(FName("UI.State.Back"));
		Subsystem->OnGameplayTagSent.Broadcast(BackTag);
	}
}

void UZWUIPlayerHUBWidget::HandleTabSelected(FName TabId)
{
	// 1. Zabezpieczenie przed brakiem konfiguracji i klikaniem tego samego
	if (!TabConfiguration || (MenuPanelsSwitcher && MenuPanelsSwitcher->IsCurrentlySwitching())) return;

	FGameplayTag StateTagToSend;

	// 2. Szukamy definicji zakładki w naszym Data Assecie
	for (const FZWUITabDefinition& TabDef : TabConfiguration->Tabs)
	{
		// Common UI zwraca nam TabId jako FName, więc porównujemy
		if (TabDef.TabTag.GetTagName() == TabId)
		{
			StateTagToSend = TabDef.StateTag;
			break;
		}
	}
	
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// CISZA! HUB jest zamykany przez State Tree (np. przez Task 'Close Panel') 
		// lub podczas zmiany zakładek. State Tree już o wszystkim wie.
		return; 
	}

	// 3. Jeśli znaleźliśmy State Tag, wysyłamy go do maszyny stanów!
	if (StateTagToSend.IsValid())
	{
		if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			// Używamy Twojego uniwersalnego nadajnika
			Subsystem->OnGameplayTagSent.Broadcast(StateTagToSend);
			
			UE_LOG(LogTemp, Log, TEXT("HUB: Zakladka zmieniona myszka. Wysylam do State Tree: %s"), *StateTagToSend.ToString());
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
	
	// Jeśli Switcher jest pusty LUB jego widget jest wyłączony -> gasimy tło
	if (!ActivePanel || !ActivePanel->IsActivated())
	{
		SetMenuBackgroundVisible(false);
		DeactivateWidget();
	}
	else
	{
		// Jeśli z jakiegoś powodu inny widget stał się aktywny, upewniamy się, 
		// czy on wymaga tła (czytamy naszą flagę)
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

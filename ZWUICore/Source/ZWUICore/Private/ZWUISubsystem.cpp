// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUISubsystem.h"

#include "ZWUIGameplayTags.h"
#include "ZWUISettings.h"
#include "ZWUILogChannels.h"
#include "ZWUIPanelDatabase.h"
#include "ZWUIPlayerHUBWidget.h"
#include "ZWUIRootLayout.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "Engine/AssetManager.h"
#include "Input/CommonUIActionRouterBase.h"
#include "Widgets/CommonActivatableWidgetContainer.h"

void UZWUISubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UZWUISettings* UISettings = GetDefault<UZWUISettings>();
	if (UISettings)
	{
		if (!UISettings->PanelRegistry.IsNull())
		{
			UZWUIPanelDatabase* LoadedRegistry = UISettings->PanelRegistry.LoadSynchronous();
			
			if (!LoadedRegistry)
			{
				UE_LOG(LogZWUICore, Warning, TEXT("ZWUIPanelDatabase not loaded properly! No UI panels will work."));
				return;
			}
			
			for (const FZWUIPanelData& PanelData : LoadedRegistry->Panels)
			{
				RegisterPanelData(PanelData);
				
#if WITH_EDITOR
			// Optional: Log what was registered for debugging
			UE_LOG(LogZWUICore, Log, TEXT("Registered UI Panel: [%s] -> [%s]"), 
					*PanelData.PanelTag.ToString(), 
					*PanelData.PanelClass.ToString());			
#endif
			}
		}
	}
}

void UZWUISubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);
	
	PlayerController = NewPlayerController;
	
	if (PlayerController)
	{
		const UZWUISettings* UISettings = GetDefault<UZWUISettings>();
		if (UISettings)
		{
			RootLayout = CreateWidget<UZWUIRootLayout>(PlayerController, UISettings->MainRootLayoutClass);
			if (RootLayout)
			{
				RootLayout->AddToViewport(0);
				
				if (UISettings && !UISettings->MainHUDClass.IsNull())
				{
					if (UClass* HUDClass = UISettings->MainHUDClass.LoadSynchronous())
					{
						UUserWidget* MainHUD = CreateWidget<UUserWidget>(PlayerController, HUDClass);
						UOverlaySlot* NewSlot = Cast<UOverlaySlot>(RootLayout->GameLayer->AddChild(MainHUD));
						NewSlot->SetPadding(FMargin());
						NewSlot->SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
						NewSlot->SetVerticalAlignment(EVerticalAlignment::VAlign_Fill);
					}
				}
				return;
			}
			UE_LOG(LogZWUICore, Warning, TEXT("Root Layout not created!"))
		}
	}
}

void UZWUISubsystem::RegisterPanelData(const FZWUIPanelData& PanelData)
{
	if (PanelData.PanelTag.IsValid() && !PanelData.PanelClass.IsNull())
	{
		RegisteredPanels.Add(PanelData.PanelTag, PanelData);
	}
}

TSoftClassPtr<UZWUIPanel> UZWUISubsystem::GetPanelClass(FGameplayTag PanelTag) const
{
	if (const FZWUIPanelData* FoundData = RegisteredPanels.Find(PanelTag))
	{
		if (!FoundData->PanelClass.IsNull())
		{
			return FoundData->PanelClass;
		}
	}
	
	return nullptr;
}

void UZWUISubsystem::RequestPanelWidget(FGameplayTag PanelTag)
{
	if (!PlayerController || !RootLayout || !PanelTag.IsValid()) return;
	
	if (PlayerHUB && !PlayerHUB->IsActivated())
	{
		PlayerHUB = nullptr; 
	}
	
	// We try to add the Menu Tab widget immediately and return if succeeded.
	if (PanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab))
	{			
		if (UZWUIPanel** FoundPanel = InstancedPanels.Find(PanelTag))
		{
			// Widget już jest w pamięci! Zakładamy, że HUB też żyje.
			if (PlayerHUB)
			{				
				PlayerHUB->OpenTabInSwitcher(*FoundPanel);
				return; // Koniec roboty!
			}
		}
	}	
	
	TSoftClassPtr<UZWUIPanel> SoftClass = GetPanelClass(PanelTag);
	if (SoftClass.IsNull()) return;
	
	TArray<FSoftObjectPath> AssetsToLoad;
	AssetsToLoad.Add(SoftClass.ToSoftObjectPath());
	
	if (PanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab))
	{
		// PlayerHUB must exist to be able to add a widget
		if (!PlayerHUB || !PlayerHUB->IsActivated())
		{
			const UZWUISettings* UISettings = GetDefault<UZWUISettings>();
			if (UISettings && !UISettings->MainHUBClass.IsNull())
			{
				AssetsToLoad.Add(UISettings->MainHUBClass.ToSoftObjectPath());
			}
		}	
	}
	
	// No other tags than those three should be here.
	if (!(PanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab) || PanelTag.MatchesTag(ZWUITags::Panel_Menu_Standalone) || PanelTag.MatchesTag(ZWUITags::Panel_Prompt)))
	{		
		UE_LOG(LogTemp, Warning, TEXT("Tag %s should not be handled by RequestPanelWidget!"), *PanelTag.ToString());
		return;
	}	
	
	// Asynchroniczne ładowanie (to zostaje bez zmian, bo napisałeś to super!)
	if (ActiveLoadHandles.Contains(PanelTag)) return;

	FStreamableManager& StreamableManager = UAssetManager::GetStreamableManager();
	TSharedPtr<FStreamableHandle> Handle = StreamableManager.RequestAsyncLoad(
		AssetsToLoad,
		FStreamableDelegate::CreateUObject(this, &UZWUISubsystem::OnPanelClassLoaded, PanelTag, SoftClass)
	);

	if (Handle.IsValid() && !Handle->HasLoadCompleted())
	{
		ActiveLoadHandles.Add(PanelTag, Handle);
	}
}

UZWUIPanel* UZWUISubsystem::GetOrCreateInstancedPanel(FGameplayTag PanelTag)
{
	if (!PanelTag.IsValid()) return nullptr;

	// 1. Jeśli panel już jest w zasobach Subsystemu - po prostu go oddajemy!
	if (UZWUIPanel** FoundPanel = InstancedPanels.Find(PanelTag))
	{
		return *FoundPanel;
	}

	// 2. Jeśli nie ma, Subsystem uroczyście go tworzy i zapisuje jako JEDYNY WŁAŚCICIEL
	TSoftClassPtr<UZWUIPanel> SoftClass = GetPanelClass(PanelTag);
	if (!SoftClass.IsNull())
	{
		// Ładujemy synchronicznie (ponieważ Eager Loading na ekranie ładowania menu to akceptowalny standard)
		if (UClass* WidgetClass = SoftClass.LoadSynchronous())
		{
			UZWUIPanel* NewPanel = CreateWidget<UZWUIPanel>(PlayerController, WidgetClass);
			NewPanel->BoundPanelTag = PanelTag;
			NewPanel->PanelIdentityTag = PanelTag;
			
			// Subsystem rejestruje to u siebie!
			InstancedPanels.Add(PanelTag, NewPanel);
			return NewPanel;
		}
	}
	return nullptr;
}

void UZWUISubsystem::ClosePanelWidget(FGameplayTag PanelTag)
{
	if (!PanelTag.IsValid()) return;
	
	if (PlayerHUB && PlayerHUB->IsActivated())
	{
		const UZWUISettings* UISettings = GetDefault<UZWUISettings>();
		if (UISettings && UISettings->MainHUBTag.IsValid())
		{
			if (PanelTag.MatchesTag(UISettings->MainHUBTag))
			{
				PlayerHUB->DeactivateWidget();
				PlayerHUB = nullptr;		
				return;
			}
		}		 
	}

	// 1. Opcja dla paneli typu Standalone / Menu, które zapisaliśmy w InstancedPanels
	if (UZWUIPanel** FoundInstanced = InstancedPanels.Find(PanelTag))
	{
		if (*FoundInstanced && (*FoundInstanced)->IsActivated())
		{
			// Wywołanie tego natywnie odpali całą procedurę zamykania Common UI:
			// Animacja zamykania -> NativeOnDeactivated -> UnregisterPanel -> RefreshInput
			(*FoundInstanced)->DeactivateWidget();
			return; // Znaleźliśmy i wyłączyliśmy, koniec roboty
		}
	}

	// 2. Fallback (dla paneli tworzonych "w locie" bez zapisywania instancji, np. niektóre Prompty)
	// Przeszukujemy naszą listę aktualnie aktywnych paneli.
	// Zakładam, że Twoja struktura w ActivePanels ma wskaźnik na Panel.
	for (int32 i = ActivePanels.Num() - 1; i >= 0; --i)
	{
		UZWUIPanel* ActivePanel = ActivePanels[i].Panel; // (Dostosuj to do swojej struktury, np. ActivePanels[i].Panel)
		
		if (ActivePanel && ActivePanel->BoundPanelTag.MatchesTagExact(PanelTag))
		{
			ActivePanel->DeactivateWidget();
			return;
		}
	}
	
	UE_LOG(LogZWUICore, Warning, TEXT("ClosePanelWidget: Nie znaleziono aktywnego panelu dla Taga %s"), *PanelTag.ToString());
}

void UZWUISubsystem::RegisterPanel(UZWUIPanel* Panel)
{
	if (Panel && ActivePanels.IndexOfByKey(Panel) == INDEX_NONE)
	{
		FZWActivePanelContext NewContext;
		NewContext.Panel = Panel;
		NewContext.PanelTag = Panel->BoundPanelTag; // Pobieramy wstrzyknięty wcześniej tag

		ActivePanels.Add(NewContext);
		RefreshInputConfig();
	}
}

void UZWUISubsystem::UnregisterPanel(UZWUIPanel* Panel)
{	
	if (!Panel) return;

	int32 Index = ActivePanels.IndexOfByKey(Panel);
	if (Index != INDEX_NONE)
	{
		ActivePanels.RemoveAt(Index);
		RefreshInputConfig();

		// SPRAWDZAMY KTO RZĄDZI:
		const UZWUISettings* Settings = GetDefault<UZWUISettings>();
		if (Settings && Settings->bIsUIStateExternallyManaged)
		{
			// CISZA! Żaden zamykający się (Unregisterowany) panel nie ma prawa 
			// sam z siebie wysyłać sygnału Back. Zrobi to tylko State Tree.
			return;
		}

		// FALLBACK DLA SAMEGO ZWUICORE:
		if (!Panel->BoundPanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab))
		{
			FGameplayTag BackTag = FGameplayTag::RequestGameplayTag(FName("UI.State.Back"));
			OnGameplayTagSent.Broadcast(BackTag);
		}
	}
}

bool UZWUISubsystem::IsPanelRegisteredByTag(FGameplayTag UITag)
{
	if (!UITag.IsValid())
	{
		return false;
	}
	
	return ActivePanels.ContainsByPredicate([&UITag](const FZWActivePanelContext& Panel)
	{
		// IMPORTANT: Choose one of the matching methods below depending on your needs!

		// Method A: Exact Match (Fastest) - Matches ONLY "UI.Panel.Inventory"
		return Panel.PanelTag == UITag; 
		
		// Method B: Hierarchical Match - Matches "UI.Panel.Inventory" and "UI.Panel.Inventory.WeaponTab"
		// return Panel.PanelTag.MatchesTag(PanelTag); 
	});
}

void UZWUISubsystem::RefreshInputConfig()
{
	UCommonUIActionRouterBase* Router = GetActionRouter();
	if (!Router) return;

	FUIInputConfig NewConfig;

	if (ActivePanels.Num() > 0)
	{
		UZWUIPanel* TopPanel = ActivePanels.Last().Panel;
		
		TOptional<FUIInputConfig> OptionalConfig = TopPanel->GetDesiredInputConfig();
        
		if (OptionalConfig.IsSet())
		{
			NewConfig = OptionalConfig.GetValue();
		}
		else
		{
			NewConfig = FUIInputConfig(ECommonInputMode::Menu, EMouseCaptureMode::NoCapture);
		}
	}
	else
	{
		NewConfig = FUIInputConfig(ECommonInputMode::Game, EMouseCaptureMode::CapturePermanently);
		NewConfig.bIgnoreLookInput = false;
		NewConfig.bIgnoreMoveInput = false;
	}
	
	Router->SetActiveUIInputConfig(NewConfig);
}

UCommonUIActionRouterBase* UZWUISubsystem::GetActionRouter() const
{
	if (ULocalPlayer* LP = GetWorld()->GetGameInstance()->GetFirstGamePlayer())
	{
		return LP->GetSubsystem<UCommonUIActionRouterBase>();
	}
	return nullptr;
}

bool UZWUISubsystem::AddPanelToLayer(FGameplayTag PanelTag, TSubclassOf<UZWUIPanel> PanelClass)
{
	if (!PanelClass) return false;
	UZWUIPanel* Panel = nullptr;
	
	if (PanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab) && PlayerHUB)
	{		
		if (UZWUIPanel** FoundPanel = InstancedPanels.Find(PanelTag))
		{
			Panel = *FoundPanel;	
		}
		else
		{
			Panel = CreateWidget<UZWUIPanel>(PlayerController, PanelClass);
			Panel->BoundPanelTag = PanelTag;
			InstancedPanels.Add(PanelTag, Panel);
		}
		
		if (Panel)
		{
			PlayerHUB->OpenTabInSwitcher(Panel);			
			return true;
		}				
	}
	
	if (PanelTag.MatchesTag(ZWUITags::Panel_Menu_Standalone) && RootLayout)
	{
		RootLayout->MenuLayer->AddWidget<UZWUIPanel>(PanelClass, 
			[PanelTag](UZWUIPanel& NewPanel)
			{
				NewPanel.BoundPanelTag = PanelTag;
			});
		return true;
	}
	
	if (PanelTag.MatchesTag(ZWUITags::Panel_Prompt) && RootLayout)
	{
		RootLayout->PromptLayer->AddWidget<UZWUIPanel>(PanelClass, 
			[PanelTag](UZWUIPanel& NewPanel)
			{
				NewPanel.BoundPanelTag = PanelTag;
			});
		return true;
	}
	
	return false;
}

void UZWUISubsystem::OnPanelWidgetClosed()
{
	//Possibly handle input refresh on widget closed.
}

void UZWUISubsystem::OnPanelClassLoaded(FGameplayTag PanelTag, TSoftClassPtr<UZWUIPanel> SoftPanelClass)
{
	ActiveLoadHandles.Remove(PanelTag);

	TSubclassOf<UZWUIPanel> WidgetClass = SoftPanelClass.Get();
	if (!WidgetClass) return;
	
	if (PanelTag.MatchesTag(ZWUITags::Panel_Menu_Tab) && !PlayerHUB)
	{
		const UZWUISettings* UISettings = GetDefault<UZWUISettings>();
		if (UISettings && UISettings->MainHUBClass.Get())
		{
			PlayerHUB = RootLayout->MenuLayer->AddWidget<UZWUIPlayerHUBWidget>(UISettings->MainHUBClass.Get());
		}
	}
	
	AddPanelToLayer(PanelTag, WidgetClass);
}

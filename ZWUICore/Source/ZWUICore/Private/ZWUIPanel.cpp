// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUIPanel.h"

#include "ZWInputConfig.h"
#include "ZWUISettings.h"
#include "ZWUISubsystem.h"
#include "Input/CommonUIInputTypes.h"

UZWUIPanel::UZWUIPanel(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Usually, main panels in a game should consume input and capture focus.
	// We set it to true by default to act as a proper menu overlay.
	bIsModal = true;
	
	bIsBackHandler = true;
}

void UZWUIPanel::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	if (const UZWUISettings* InputSettings = GetDefault<UZWUISettings>())
	{
		if (UZWInputConfig* LoadedConfig = InputSettings->InputConfig.LoadSynchronous())
		{
			for (const FZWInputAction& GenericInputAction : LoadedConfig->GenericInputActions)
			{
				FGameplayTag ActionTag = GenericInputAction.InputTag;
				if (ActionTag.IsValid())
				{
					if (UInputAction* InputAction = GenericInputAction.InputAction.LoadSynchronous())
					{
						FSimpleDelegate Delegate;
						Delegate.BindWeakLambda(this, [this, ActionTag]()
						{
							// BROADCAST: Robimy dokładnie to samo co Twój ZWInputComponent!
							// Żadnego sztywnego State Tree. Kto nasłuchuje w grze, ten łapie Taga.
							if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
							{
								// Załóżmy, że dodałeś taki delegat w Subsystemie:
								// FOnPanelClosedDelegate OnGenericUIActionTriggered;
								Subsystem->OnGameplayTagSent.Broadcast(ActionTag);
						
								// ^ (Możesz tu użyć OnPanelClosed, albo stworzyć nowy delegat np. OnGenericUIAction)
							}
						});

						FBindUIActionArgs Args(InputAction, Delegate);
				
						// Wymuszamy tryb Menu, żeby Common UI elegancko to przetworzyło
						Args.InputMode = ECommonInputMode::Menu; 
				
						// Ukrywamy to z Action Baru (żeby nie generowało syfu na ekranie dla każdej akcji)
						Args.bDisplayInActionBar = false; 
				
						RegisterUIActionBinding(Args);
					}
				}
			}
		}		
	}
}

void UZWUIPanel::NativeOnActivated()
{
	Super::NativeOnActivated();
    
	if (bRequiresInput && GetOwningLocalPlayer())
	{
		if (UZWUISubsystem* UISubsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			UISubsystem->RegisterPanel(this);
		}	
	}
	
	// Here you can add global behaviors for when ANY panel opens
	// (e.g., playing a default 'swoosh' sound or adding background blur)
}

void UZWUIPanel::NativeOnDeactivated()
{
	Super::NativeOnDeactivated();
	
	if (bRequiresInput && GetOwningLocalPlayer())
	{
		if (UZWUISubsystem* UISubsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			UISubsystem->UnregisterPanel(this);
		}	
	}
	// Global cleanup when a panel closes
}

UWidget* UZWUIPanel::NativeGetDesiredFocusTarget() const
{
	// Allows overriding which specific button/element should get focus when this panel opens.
	// By default, it defers to the Common UI logic.
	return Super::NativeGetDesiredFocusTarget();
}

bool UZWUIPanel::NativeOnHandleBackAction()
{
	const UZWUISettings* Settings = GetDefault<UZWUISettings>();

	// 1. SPRAWDZAMY KTO RZĄDZI ZAMYKANIEM:
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// Panel ma dyrektora (State Tree). 
		// Robimy Broadcast i CELOWO nie zamykamy panelu!
		if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			// Wysyłamy Tag, który użytkownik ustawił w Project Settings (np. UI.Action.Back)
			Subsystem->OnGameplayTagSent.Broadcast(Settings->ExternalCloseTag);
		}

		// Zwracamy true - mówimy systemowi Common UI: 
		// "Zjedliśmy ten input, obsłużyliśmy go, ale nie wyłączaj nas. Zrobi to State Tree."
		return true; 
	}

	// 2. JEŚLI PLUGIN DZIAŁA SAMODZIELNIE (Tylko ZWUICore):
	// Używamy natywnego zachowania Common UI.
	// Pod maską, Super::NativeOnHandleBackAction() wywoła po prostu DeactivateWidget() i zwróci true.
	return Super::NativeOnHandleBackAction();
}

TOptional<FUIInputConfig> UZWUIPanel::GetDesiredInputConfig() const
{
	// TODO: Add option to specify desired input config
	return Super::GetDesiredInputConfig();
}

//void UZWUIPanel::HandleBackAction()
//{
	// Deactivate the widget, which returns focus to the previous UI or the game
//	DeactivateWidget();
//}

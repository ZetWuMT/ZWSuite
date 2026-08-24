// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUIPanel.h"

#include "Input/CommonUIInputTypes.h"
#include "ZWInputConfig.h"
#include "ZWUISettings.h"
#include "ZWUISubsystem.h"

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
							// Actions the panel declared for the action bar are dispatched to the panel
							// so the widget can react (see HandleActionTag). The tag is still broadcast
							// afterwards for the State Tree / any other listener.
							if (ActionBarTags.Contains(ActionTag))
							{
								HandleActionTag(ActionTag);
							}

							if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
							{
								Subsystem->OnGameplayTagSent.Broadcast(ActionTag);
							}
						});

						FBindUIActionArgs Args(InputAction, Delegate);
				
						// Force Menu mode so Common UI processes it gracefully
						Args.InputMode = ECommonInputMode::Menu; 
				
						// Only actions explicitly listed in ActionBarTags show up in the action bar;
						// everything else stays hidden so it does not generate clutter.
						Args.bDisplayInActionBar = ActionBarTags.Contains(ActionTag); 
				
						RegisterUIActionBinding(Args);
					}
				}
			}
		}		
	}
}

void UZWUIPanel::HandleActionTag_Implementation(FGameplayTag InputTag)
{
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

	// 1. CHECK WHO RUNS THE CLOSING:
	if (Settings && Settings->bIsUIStateExternallyManaged)
	{
		// The panel has a director (State Tree). 
		// We broadcast and INTENTIONALLY do not close the panel!
		if (UZWUISubsystem* Subsystem = GetOwningLocalPlayer()->GetSubsystem<UZWUISubsystem>())
		{
			// Send the Tag that the user set in Project Settings (e.g. UI.Action.Back)
			Subsystem->OnGameplayTagSent.Broadcast(Settings->ExternalCloseTag);
		}

		// Return true - we tell the Common UI system:
		// "We ate this input, handled it, but do not disable us. The State Tree will do it."
		return true; 
	}

	// 2. IF THE PLUGIN WORKS STANDALONE (Only ZWUICore):
	// We use Common UI's native behavior.
	// Under the hood, Super::NativeOnHandleBackAction() simply calls DeactivateWidget() and returns true.
	return Super::NativeOnHandleBackAction();
}

TOptional<FUIInputConfig> UZWUIPanel::GetDesiredInputConfig() const
{
	// TODO: Add option to specify desired input config
	return Super::GetDesiredInputConfig();
}
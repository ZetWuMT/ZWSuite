// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInputComponent.h"
#include "GameplayTagContainer.h"
#include "ZWInputSettings.h"


// Sets default values for this component's properties
UZWInputComponent::UZWInputComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}

void UZWInputComponent::InitializeInput()
{
	CachedConfig = GetInputConfigAsset();
	if (CachedConfig)
	{
		for (const FZWInputAction& GenericInputAction : CachedConfig->GenericInputActions)
		{
			FGameplayTag ActionTag = GenericInputAction.InputTag;
			if (ActionTag.IsValid())
			{
				if (UInputAction* InputAction = GenericInputAction.InputAction.LoadSynchronous())
				{
					BindAction(InputAction, GenericInputAction.TriggerEvent, this, &UZWInputComponent::HandleGenericInput, ActionTag);
				}
			}
		}
	}
}

void UZWInputComponent::HandleGenericInput(const FInputActionValue& ActionValue, FGameplayTag InputTag)
{
	OnInputTagTriggered.Broadcast(InputTag, ActionValue);
	OnInputTagSimpleTriggered.Broadcast(InputTag);	
}

const UZWInputConfig* UZWInputComponent::GetInputConfigAsset()
{
	if (const UZWInputSettings* InputSettings = GetDefault<UZWInputSettings>())
	{
		UZWInputConfig* LoadedConfig = InputSettings->InputConfig.LoadSynchronous();
		
		if (LoadedConfig)
		{
			return LoadedConfig;
		}
		UE_LOG(LogTemp, Warning, TEXT("ZWInputComponent: UIInputConfig is missing!"));
		return nullptr;
	}
	UE_LOG(LogTemp, Warning, TEXT("ZWInputComponent: UISettings are missing!"));
	return nullptr;
}

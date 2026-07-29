// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWUIStateTreeSubsystem.h"

#include "StateTreeExecutionContext.h"
#include "ZWUIStateTreeSettings.h"
#include "ZWUISubsystem.h" // We need this to bind it to the context

const UStateTree* UZWUIStateTreeSubsystem::GetStateTreeAsset() const
{
	const UZWUIStateTreeSettings* Settings = GetDefault<UZWUIStateTreeSettings>();
	if (Settings && !Settings->DefaultUIStateTree.IsNull())
	{
		return Settings->DefaultUIStateTree.LoadSynchronous();
	}
	return nullptr;
}

void UZWUIStateTreeSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	// Baza (UZWStateTreeSubsystemBase) zajmuje się (re)startem drzewa - my dokładamy tylko to,
	// co jest specyficzne dla UI: podpięcie się pod broadcast tagów z UZWUISubsystem.
	Super::PlayerControllerChanged(NewPlayerController);

	if (NewPlayerController)
	{
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->OnGameplayTagSent.AddUObject(this, &UZWUIStateTreeSubsystem::ProcessUITag);
			}
		}
	}
}

void UZWUIStateTreeSubsystem::BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset)
{
	if (!TreeAsset) return;

	// Retrieve the context descriptors built by the compiler
	TConstArrayView<FStateTreeExternalDataDesc> ContextDescs = TreeAsset->GetContextDataDescs();

	// Inject the ZWUISubsystem required by our ZWUIStateTreeSchema
	for (const FStateTreeExternalDataDesc& Desc : ContextDescs)
	{
		if (Desc.Struct && Desc.Struct->IsChildOf(UZWUISubsystem::StaticClass()))
		{
			// Fetch the UI subsystem from the local player
			if (ULocalPlayer* LP = GetLocalPlayer())
			{
				if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
				{
					FStateTreeDataView UIView(UISubsystem);
					Context.SetContextData(Desc.Handle, UIView);
				}
			}
			break; 
		}
	}
}

void UZWUIStateTreeSubsystem::ProcessUITag(FGameplayTag UITag)
{
	SendStateTreeEvent(UITag);
}

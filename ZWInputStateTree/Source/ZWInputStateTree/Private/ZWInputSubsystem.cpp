// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInputSubsystem.h"

#include "EnhancedInputSubsystems.h"
#include "StateTreeExecutionContext.h"
#include "ZWInputComponent.h"
#include "ZWInputStateTreeSettings.h"

const UStateTree* UZWInputSubsystem::GetStateTreeAsset() const
{
	const UZWInputStateTreeSettings* Settings = GetDefault<UZWInputStateTreeSettings>();
	if (Settings && !Settings->DefaultInputStateTree.IsNull())
	{
		// Load the asset synchronously from the path (only once, at player startup)
		return Settings->DefaultInputStateTree.LoadSynchronous();
	}
	return nullptr;
}

void UZWInputSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	// The base (UZWStateTreeSubsystemBase) handles (re)starting the tree - we only add what is
	// specific to Input: subscribing to the tag broadcast from ZWInputComponent.
	Super::PlayerControllerChanged(NewPlayerController);

	if (NewPlayerController)
	{
		if (UZWInputComponent* InputComponent = NewPlayerController->GetComponentByClass<UZWInputComponent>())
		{
			InputComponent->OnInputTagTriggered.AddUObject(this, &UZWInputSubsystem::ProcessInputTag);
		}
	}
}

void UZWInputSubsystem::ProcessInputTag(FGameplayTag InputTag, const FInputActionValue& InputActionValue)
{
	SendStateTreeEvent(InputTag);

	OnInputTagDelegate.Broadcast(InputTag);
}

void UZWInputSubsystem::PushInputContext(const UInputMappingContext* IMC, int32 Priority)
{
	if (!IMC) return;

	if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		EISubsystem->AddMappingContext(IMC, Priority);
		
		// Optionally track what we added
		ActiveContexts.AddUnique(IMC);
	}
}

void UZWInputSubsystem::PopInputContext(const UInputMappingContext* IMC)
{
	if (!IMC) return;

	if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = GetLocalPlayer()->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		EISubsystem->RemoveMappingContext(IMC);
		ActiveContexts.Remove(IMC);
	}
}

void UZWInputSubsystem::BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset)
{
	if (!TreeAsset) return;

	// CRITICAL CHANGE: We get the descriptors from the COMPILED TREE
	// These descriptors already have generated, valid Handles!
	TConstArrayView<FStateTreeExternalDataDesc> ContextDescs = TreeAsset->GetContextDataDescs();

	for (const FStateTreeExternalDataDesc& Desc : ContextDescs)
	{
		// Look for the descriptor that expects our Subsystem
		if (Desc.Struct && Desc.Struct->IsChildOf(UZWInputSubsystem::StaticClass()))
		{
			FStateTreeDataView SubsystemView(this);
			Context.SetContextData(Desc.Handle, SubsystemView);
			
		}
		else if (Desc.Struct && Desc.Struct->IsChildOf(APlayerController::StaticClass()))
		{
			if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
			{
				FStateTreeDataView PCView(PC);
				
				Context.SetContextData(Desc.Handle, PCView);
			}
		}
		else if (Desc.Struct && Desc.Struct->IsChildOf(AActor::StaticClass()))
		{
			// Since the Subsystem lives on the LocalPlayer, we can easily reach the Pawn
			if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
			{
				if (APawn* PlayerPawn = PC->GetPawn())
				{
					// Create a data view on our Pawn
					FStateTreeDataView ActorView(PlayerPawn);
					
					// Inject the Pawn into the State Tree under this specific Handle!
					Context.SetContextData(Desc.Handle, ActorView);
				}
			}
		}
	}
}

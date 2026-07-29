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
		// Ładujemy synchronicznie asset ze ścieżki (tylko raz, przy starcie gracza)
		return Settings->DefaultInputStateTree.LoadSynchronous();
	}
	return nullptr;
}

void UZWInputSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	// Baza (UZWStateTreeSubsystemBase) zajmuje się (re)startem drzewa - my dokładamy tylko to,
	// co jest specyficzne dla Inputu: podpięcie się pod broadcast tagów z ZWInputComponent.
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
		
		// Opcjonalnie śledzimy, co dodaliśmy
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

	// KRYTYCZNA ZMIANA: Pobieramy deskryptory ze SKOMPILOWANEGO DRZEWA
	// Te deskryptory mają już wygenerowane, ważne Handle!
	TConstArrayView<FStateTreeExternalDataDesc> ContextDescs = TreeAsset->GetContextDataDescs();

	for (const FStateTreeExternalDataDesc& Desc : ContextDescs)
	{
		// Szukamy tego deskryptora, który oczekuje naszego Subsystemu
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
			// Jako że Subsystem żyje na LocalPlayerze, łatwo możemy dobrać się do Pawna
			if (APlayerController* PC = GetLocalPlayer()->GetPlayerController(GetWorld()))
			{
				if (APawn* PlayerPawn = PC->GetPawn())
				{
					// Tworzymy widok danych na naszego Pawna
					FStateTreeDataView ActorView(PlayerPawn);
					
					// Wstrzykujemy Pawna do State Tree pod ten konkretny Handle!
					Context.SetContextData(Desc.Handle, ActorView);
				}
			}
		}
	}
}

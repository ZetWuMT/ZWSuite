// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInputSubsystem.h"

#include "EnhancedInputSubsystems.h"
#include "StateTreeExecutionContext.h"
#include "StateTreeExecutionTypes.h"
#include "ZWInputComponent.h"
#include "ZWInputStateTreeSettings.h"

void UZWInputSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	
	const UZWInputStateTreeSettings* Settings = GetDefault<UZWInputStateTreeSettings>();
	if (Settings && !Settings->DefaultInputStateTree.IsNull())
	{
		// Ładujemy synchronicznie asset ze ścieżki (tylko raz, przy starcie gracza)
		UStateTree* LoadedTree = Settings->DefaultInputStateTree.LoadSynchronous();
		
		// Przypisujemy do naszej referencji, na której pracuje reszta klasy
		StateTreeRef.SetStateTree(LoadedTree);
	}
	
	if (!StateTreeRef.IsValid()) return;

	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	if (!TreeAsset) return;
	
	StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());
}

void UZWInputSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);	
    
	// Odpalamy drzewo tylko wtedy, gdy gracz faktycznie DOSTAJE kontroler
	if (NewPlayerController)
	{
		if (UZWInputComponent* InputComponent = NewPlayerController->GetComponentByClass<UZWInputComponent>())
		{
			InputComponent->OnInputTagTriggered.AddUObject(this, &UZWInputSubsystem::ProcessInputTag);
		}
		
		const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
		if (TreeAsset && StateTreeInstanceData.Num() > 0)
		{
			FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
			BindContextData(Context, TreeAsset);

			Context.Start(); 
		}
	}
	else
	{
		// Opcjonalnie: Gdy gracz traci kontroler, moglibyśmy tu wywołać Context.Stop(),
		// żeby drzewo przestało nasłuchiwać i zresetowało swoje stany.
	}
}

void UZWInputSubsystem::Tick(float DeltaTime)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
    
	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset); // <-- Taski w Ticku będą miały dostęp do Subsystemu
        
		Context.Tick(DeltaTime);
	}
}

TStatId UZWInputSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZWInputSubsystem, STATGROUP_Tickables);
}

bool UZWInputSubsystem::IsTickable() const
{
	return !HasAnyFlags(RF_ClassDefaultObject);
}

void UZWInputSubsystem::ProcessInputTag(FGameplayTag InputTag, const FInputActionValue& InputActionValue)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
    
	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);
		
		Context.SendEvent(InputTag);
	}
	
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

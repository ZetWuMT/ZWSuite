#include "ZWActionManagerComponent.h"

#include "InputActionValue.h"
#include "ZWGameplayAction.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UZWActionManagerComponent::UZWActionManagerComponent()
{
	// Menedżer nasłuchuje tylko na strzały z Inputu. Nie musi tykać co klatkę!
	PrimaryComponentTick.bCanEverTick = false;
}

void UZWActionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Inicjalizacja domyślnych akcji (np. Skok, Interakcja, Bieg), 
	// które dodasz swojemu graczowi w okienku Details w edytorze.
	for (TSubclassOf<UZWGameplayAction> ActionClass : DefaultActions)
	{
		GrantAction(ActionClass);
	}
}

void UZWActionManagerComponent::GrantAction(TSubclassOf<UZWGameplayAction> ActionClass)
{
	if (!ActionClass) return;

	// Zabezpieczenie przed podwójnym nadaniem tej samej akcji.
	// Nie chcemy, żeby gracz miał w kieszeni 5 instancji akcji "Zrób Zdjęcie".
	for (UZWGameplayAction* ExistingAction : GrantedActions)
	{
		if (ExistingAction && ExistingAction->IsA(ActionClass))
		{
			// Gracz już potrafi to robić. Przerywamy.
			return; 
		}
	}

	// Tworzymy nową instancję akcji. Outerem (właścicielem) jest nasz Menedżer.
	UZWGameplayAction* NewAction = NewObject<UZWGameplayAction>(this, ActionClass);
	
	if (NewAction)
	{
		GrantedActions.Add(NewAction);
	}
}

void UZWActionManagerComponent::RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass)
{
	if (!ActionClass) return;

	// Szukamy akcji do usunięcia. Iterujemy od końca (typowe przy usuwaniu z TArray).
	for (int32 i = GrantedActions.Num() - 1; i >= 0; --i)
	{
		UZWGameplayAction* Action = GrantedActions[i];
		if (Action && Action->IsA(ActionClass))
		{
			GrantedActions.RemoveAt(i);
			
			// Nie musimy robić ręcznego "Destroy". Jak tylko wyrzucimy obiekt z tablicy GrantedActions
			// (która ma makro UPROPERTY), system Garbage Collection Unreala sam go zniszczy w tle.
			break;
		}
	}
}

void UZWActionManagerComponent::HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue)
{
	if (!InputTag.IsValid()) return;

	// Przeszukujemy naszą "kieszeń" z akcjami.
	for (UZWGameplayAction* Action : GrantedActions)
	{
		// Jeśli Tag z Action Routera zgadza się z Tagiem ustawionym w Blueprincie akcji:
		if (Action && Action->TriggerTag.MatchesTagExact(InputTag))
		{
			if (APawn* Avatar = Cast<APawn>(GetOwner()))
			{
				APlayerController* PC = Cast<APlayerController>(Avatar->GetController());
				
				// ODPALAMY LOGIKĘ! 
				// (To wywoła węzły podpięte pod Event Execute Action wewnątrz Twojego Blueprinta)
				Action->ExecuteAction(PC, Avatar);
			}
		}
	}
}
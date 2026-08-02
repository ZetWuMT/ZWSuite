#include "ZWActionManagerComponent.h"

#include "InputActionValue.h"
#include "ZWGameplayAction.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

UZWActionManagerComponent::UZWActionManagerComponent()
{
	// The manager only listens to Input fires. It does not need to tick every frame!
	PrimaryComponentTick.bCanEverTick = false;
}

void UZWActionManagerComponent::BeginPlay()
{
	Super::BeginPlay();

	// Initialization of the default actions (e.g. Jump, Interact, Run),
	// which you add to your player in the Details window in the editor.
	for (TSubclassOf<UZWGameplayAction> ActionClass : DefaultActions)
	{
		GrantAction(ActionClass);
	}
}

void UZWActionManagerComponent::GrantAction(TSubclassOf<UZWGameplayAction> ActionClass)
{
	if (!ActionClass) return;

	// Guard against granting the same action twice.
	// We do not want the player to have 5 instances of the "Take Photo" action in their pocket.
	for (UZWGameplayAction* ExistingAction : GrantedActions)
	{
		if (ExistingAction && ExistingAction->IsA(ActionClass))
		{
			// The player can already do this. Stop.
			return; 
		}
	}

	// Create a new action instance. The Outer (owner) is our Manager.
	UZWGameplayAction* NewAction = NewObject<UZWGameplayAction>(this, ActionClass);
	
	if (NewAction)
	{
		GrantedActions.Add(NewAction);
	}
}

void UZWActionManagerComponent::RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass)
{
	if (!ActionClass) return;

	// Look for the action to remove. We iterate from the end (typical when removing from a TArray).
	for (int32 i = GrantedActions.Num() - 1; i >= 0; --i)
	{
		UZWGameplayAction* Action = GrantedActions[i];
		if (Action && Action->IsA(ActionClass))
		{
			GrantedActions.RemoveAt(i);
			
			// We do not need a manual "Destroy". As soon as we remove the object from the GrantedActions
			// array (which has the UPROPERTY macro), Unreal's Garbage Collection destroys it in the background.
			break;
		}
	}
}

void UZWActionManagerComponent::HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue)
{
	if (!InputTag.IsValid()) return;

	// Search through our "pocket" of actions.
	for (UZWGameplayAction* Action : GrantedActions)
	{
		// If the Tag from the Action Router matches the Tag set in the action's Blueprint:
		if (Action && Action->TriggerTag.MatchesTagExact(InputTag))
		{
			if (APawn* Avatar = Cast<APawn>(GetOwner()))
			{
				APlayerController* PC = Cast<APlayerController>(Avatar->GetController());
				
				// FIRE THE LOGIC! 
				// (This will invoke the nodes hooked up to the Execute Action event inside your Blueprint)
				Action->ExecuteAction(PC, Avatar);
			}
		}
	}
}
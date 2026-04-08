#include "ZWStateTreeTasks_GameplayActions.h"
#include "ZWActionManagerComponent.h" // Z drugiego pluginu!
#include "StateTreeExecutionContext.h"

// =====================================================================
// TASK 1: ADD GAMEPLAY ACTION
// =====================================================================

EStateTreeRunStatus FZWStateTreeTask_AddGameplayAction::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.TargetActor && InstanceData.ActionClass)
	{
		if (UZWActionManagerComponent* ActionManager = InstanceData.TargetActor->GetComponentByClass<UZWActionManagerComponent>())
		{
			ActionManager->GrantAction(InstanceData.ActionClass);
		}
	}

	// Task po prostu ustawia logikę i kontynuuje działanie stanu
	return EStateTreeRunStatus::Running;
}

void FZWStateTreeTask_AddGameplayAction::ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	// Sprzątamy tylko wtedy, gdy flaga bRemoveOnExit jest zaznaczona
	if (InstanceData.bRemoveOnExit && InstanceData.TargetActor && InstanceData.ActionClass)
	{
		if (UZWActionManagerComponent* ActionManager = InstanceData.TargetActor->GetComponentByClass<UZWActionManagerComponent>())
		{
			ActionManager->RemoveAction(InstanceData.ActionClass);
		}
	}
}

// =====================================================================
// TASK 2: REMOVE GAMEPLAY ACTION
// =====================================================================

EStateTreeRunStatus FZWStateTreeTask_RemoveGameplayAction::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	FInstanceDataType& InstanceData = Context.GetInstanceData(*this);

	if (InstanceData.TargetActor && InstanceData.ActionClass)
	{
		if (UZWActionManagerComponent* ActionManager = InstanceData.TargetActor->GetComponentByClass<UZWActionManagerComponent>())
		{
			ActionManager->RemoveAction(InstanceData.ActionClass);
		}
	}

	return EStateTreeRunStatus::Running;
}
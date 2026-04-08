#include "Tasks/ZWUIStateTreeTask_AddInputMappingContext.h"

#include "EnhancedInputSubsystems.h"
#include "StateTreeLinker.h"


EStateTreeRunStatus FZWUIStateTreeTask_AddInputMappingContext::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Przez Ownera wyciągamy lokalnego gracza
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				EISubsystem->AddMappingContext(InputMappingContext, Priority);
				return EStateTreeRunStatus::Running;
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}

void FZWUIStateTreeTask_AddInputMappingContext::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (!bRevertOnExit) return;

	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Przez Ownera wyciągamy lokalnego gracza
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			if (UEnhancedInputLocalPlayerSubsystem* EISubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
			{
				EISubsystem->RemoveMappingContext(InputMappingContext);
			}
		}
	}
}

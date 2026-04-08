#include "Tasks/ZWUIStateTreeTask_ManageUI.h"

#include "StateTreeLinker.h"


EStateTreeRunStatus FZWUIStateTreeTask_ManageUI::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Przez Ownera wyciągamy lokalnego gracza
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			// 3. Mając gracza, prosimy o ZWUISubsystem!
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->RequestPanelWidget(PanelTag);				
				return EStateTreeRunStatus::Running;
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}

void FZWUIStateTreeTask_ManageUI::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{	
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->ClosePanelWidget(PanelTag);
			}
		}
	}
}

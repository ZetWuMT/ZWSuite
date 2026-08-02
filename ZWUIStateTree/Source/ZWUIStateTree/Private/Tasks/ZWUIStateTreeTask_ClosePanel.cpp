#include "Tasks/ZWUIStateTreeTask_ClosePanel.h"

#include "StateTreeLinker.h"
#include "ZWUISubsystem.h"


EStateTreeRunStatus FZWUIStateTreeTask_ClosePanel::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (bCloseOnExit) return EStateTreeRunStatus::Running;
	
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Get the local player through the Owner
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			// 3. Having the player, ask for the ZWUISubsystem!
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->ClosePanelWidget(PanelTag);				
				return EStateTreeRunStatus::Succeeded;
			}
		}
	}

	return EStateTreeRunStatus::Failed;
}

void FZWUIStateTreeTask_ClosePanel::ExitState(FStateTreeExecutionContext& Context,
	const FStateTreeTransitionResult& Transition) const
{
	if (!bCloseOnExit) return;
	
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Get the local player through the Owner
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			// 3. Having the player, ask for the ZWUISubsystem!
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->ClosePanelWidget(PanelTag);
			}
		}
	}
}

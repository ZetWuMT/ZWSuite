#include "Tasks/ZWUIStateTreeTask_ClosePanel.h"

#include "StateTreeLinker.h"
#include "ZWUISubsystem.h"


EStateTreeRunStatus FZWUIStateTreeTask_ClosePanel::EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const
{
	if (bCloseOnExit) return EStateTreeRunStatus::Running;
	
	if (ULocalPlayerSubsystem* OwnerSubsystem = Cast<ULocalPlayerSubsystem>(Context.GetOwner()))
	{
		// 2. Przez Ownera wyciągamy lokalnego gracza
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			// 3. Mając gracza, prosimy o ZWUISubsystem!
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
		// 2. Przez Ownera wyciągamy lokalnego gracza
		if (ULocalPlayer* LP = OwnerSubsystem->GetLocalPlayer())
		{
			// 3. Mając gracza, prosimy o ZWUISubsystem!
			if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
			{
				UISubsystem->ClosePanelWidget(PanelTag);
			}
		}
	}
}

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "ZWUIStateTreeTask_ClosePanel.generated.h"

// InstanceData is needed so the Task can compile in the engine
USTRUCT()
struct ZWUISTATETREE_API FZWClosePanel_InstanceData
{
	GENERATED_BODY()
	

};

USTRUCT(meta = (DisplayName="Close UI Panel"))
struct ZWUISTATETREE_API FZWUIStateTreeTask_ClosePanel : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FZWClosePanel_InstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	UPROPERTY(EditAnywhere, Category = "UI")
	FGameplayTag PanelTag;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	bool bCloseOnExit = true;
	
	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

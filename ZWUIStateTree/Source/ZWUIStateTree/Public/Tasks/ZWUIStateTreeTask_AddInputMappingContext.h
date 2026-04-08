#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "ZWUIStateTreeTask_AddInputMappingContext.generated.h"

// InstanceData is needed so the Task can compile in the engine
USTRUCT()
struct ZWUISTATETREE_API FZWAddInputMappingContext_InstanceData
{
	GENERATED_BODY()
	

};

USTRUCT(meta = (DisplayName="Add Input Mapping Context"))
struct ZWUISTATETREE_API FZWUIStateTreeTask_AddInputMappingContext : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FZWAddInputMappingContext_InstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	UPROPERTY(EditAnywhere, Category = "UI")
	TObjectPtr<class UInputMappingContext> InputMappingContext = nullptr;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	bool bRevertOnExit = true;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	int32 Priority = 1;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "StateTreeExecutionContext.h"
#include "GameplayTagContainer.h"
#include "ZWUISubsystem.h"
#include "ZWUIStateTreeTask_ManageUI.generated.h"

UENUM(BlueprintType)
enum class EZWUIOperation : uint8
{
	Open   UMETA(DisplayName = "Open Panel"),
	Close  UMETA(DisplayName = "Close Panel")
};

// InstanceData is needed so the Task can compile in the engine
USTRUCT()
struct ZWUISTATETREE_API FZWManageUI_InstanceData
{
	GENERATED_BODY()
	

};

USTRUCT(meta = (DisplayName="Manage UI Panel"))
struct ZWUISTATETREE_API FZWUIStateTreeTask_ManageUI : public FStateTreeTaskCommonBase
{
	GENERATED_BODY()
	
	using FInstanceDataType = FZWManageUI_InstanceData;
	
	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	// --- Task Configuration ---
	//UPROPERTY(EditAnywhere, Category = "UI")
	//EZWUIOperation Operation = EZWUIOperation::Open;

	UPROPERTY(EditAnywhere, Category = "UI")
	FGameplayTag PanelTag;
	
	UPROPERTY(EditAnywhere, Category = "UI")
	bool bRevertOnExit = true;

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

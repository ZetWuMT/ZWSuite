#pragma once

#include "CoreMinimal.h"
#include "StateTreeTaskBase.h"
#include "ZWGameplayAction.h" // Z drugiego pluginu!
#include "ZWStateTreeTasks_GameplayActions.generated.h"

// =====================================================================
// TASK 1: ADD GAMEPLAY ACTION
// =====================================================================

USTRUCT()
struct FZWStateTreeTask_AddGameplayAction_InstanceData
{
	GENERATED_BODY()

	// Aktor, któremu nadajemy akcję (zazwyczaj podpinamy tu gracza z Context Data)
	UPROPERTY(EditAnywhere, Category = "Input")
	AActor* TargetActor = nullptr;

	// Jaką akcję nadajemy?
	UPROPERTY(EditAnywhere, Category = "Parameter")
	TSubclassOf<UZWGameplayAction> ActionClass;

	// Czy zdjąć tę akcję przy wyjściu ze stanu?
	UPROPERTY(EditAnywhere, Category = "Parameter")
	bool bRemoveOnExit = true;
};

USTRUCT(meta = (DisplayName = "Add Gameplay Action", Category = "ZW Actions"))
struct ZWINPUTSTATETREE_API FZWStateTreeTask_AddGameplayAction : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZWStateTreeTask_AddGameplayAction_InstanceData;

	FZWStateTreeTask_AddGameplayAction() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
	virtual void ExitState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};

// =====================================================================
// TASK 2: REMOVE GAMEPLAY ACTION
// =====================================================================

USTRUCT()
struct FZWStateTreeTask_RemoveGameplayAction_InstanceData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<AActor> TargetActor = nullptr;

	UPROPERTY(EditAnywhere, Category = "Parameter")
	TSubclassOf<UZWGameplayAction> ActionClass;
};

USTRUCT(meta = (DisplayName = "Remove Gameplay Action", Category = "ZW Actions"))
struct ZWINPUTSTATETREE_API FZWStateTreeTask_RemoveGameplayAction : public FStateTreeTaskBase
{
	GENERATED_BODY()

	using FInstanceDataType = FZWStateTreeTask_RemoveGameplayAction_InstanceData;

	FZWStateTreeTask_RemoveGameplayAction() = default;

	virtual const UStruct* GetInstanceDataType() const override { return FInstanceDataType::StaticStruct(); }

	virtual EStateTreeRunStatus EnterState(FStateTreeExecutionContext& Context, const FStateTreeTransitionResult& Transition) const override;
};
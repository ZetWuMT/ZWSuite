// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWStateTreeSubsystemBase.h"
#include "ZWUIStateTreeSubsystem.generated.h"

/**
 * Subsystem responsible for managing and executing the UI-specific State Tree.
 * It acts as the brain for UI flow, completely decoupled from Input logic.
 *
 * All of the generic "own a tree, tick it, (re)start it when we get a PlayerController"
 * plumbing now lives in UZWStateTreeSubsystemBase (ZWStateTreeCore plugin). This class only
 * adds what is specific to the UI schema: reading the tree asset from
 * UZWUIStateTreeSettings, binding UZWUISubsystem context data, and forwarding
 * UZWUISubsystem::OnGameplayTagSent into the tree as events.
 *
 * NOTE (migration): the old public "StartUITree()" helper has been removed - it is now
 * inherited from the base class as "StartStateTree()". If any Blueprint/C++ code calls
 * StartUITree(), update it to call StartStateTree() instead.
 */
UCLASS()
class ZWUISTATETREE_API UZWUIStateTreeSubsystem : public UZWStateTreeSubsystemBase
{
	GENERATED_BODY()
	
public:
	//~ Begin ULocalPlayerSubsystem Interface
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~ End ULocalPlayerSubsystem Interface

	/** * Injects an event tag into the UI State Tree queue.
	 * @param UITag The gameplay tag representing the UI event (e.g., "UI.Event.Inventory.Closed")
	 */
	UFUNCTION(BlueprintCallable)
	void ProcessUITag(FGameplayTag UITag);

protected:
	//~ Begin UZWStateTreeSubsystemBase Interface
	virtual const UStateTree* GetStateTreeAsset() const override;
	virtual void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset) override;
	//~ End UZWStateTreeSubsystemBase Interface
};

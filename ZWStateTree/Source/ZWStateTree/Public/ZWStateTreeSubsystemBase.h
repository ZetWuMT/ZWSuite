// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StateTreeInstanceData.h"
#include "StateTreeReference.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ZWStateTreeSubsystemBase.generated.h"

class UStateTree;
struct FStateTreeExecutionContext;

/**
 * Shared infrastructure for ZW LocalPlayerSubsystems that each drive a single, independent
 * State Tree instance (e.g. UZWInputSubsystem in ZWInputStateTree, UZWUIStateTreeSubsystem in
 * ZWUIStateTree).
 *
 * Before this refactor, every such subsystem duplicated the exact same boilerplate:
 *  - owning an FStateTreeReference / FStateTreeInstanceData pair
 *  - ticking the tree every frame via FTickableGameObject
 *  - (re)starting the tree once the local player receives a PlayerController
 *  - building an FStateTreeExecutionContext before every Start/Tick/SendEvent call
 *
 * This class owns that boilerplate once. A derived class only has to answer two questions:
 *  1) Which State Tree asset should I run?            -> GetStateTreeAsset()
 *  2) How do I bind my schema's external context data? -> BindContextData()
 *
 * Design note: this lives in its own lightweight plugin (ZWStateTreeCore), not in ZWCore.
 * ZWCore is the common ancestor of both the ZWInput family and the ZWUI family, and neither
 * "bare" ZWInput nor "bare" ZWUICore need StateTreeModule as a dependency. Only the optional
 * *StateTree plugins (ZWInputStateTree, ZWUIStateTree) depend on ZWStateTreeCore, so projects
 * that don't use State Tree at all never pull it in.
 */
UCLASS(Abstract)
class ZWSTATETREE_API UZWStateTreeSubsystemBase : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()

public:
	//~ Begin USubsystem Interface
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	//~ End USubsystem Interface

	//~ Begin ULocalPlayerSubsystem Interface
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~ End ULocalPlayerSubsystem Interface

	//~ Begin FTickableGameObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	//~ End FTickableGameObject Interface

	/**
	 * Rebuilds the instance data from the tree's defaults and (re)starts execution.
	 * Use this if you need to explicitly (re)start the tree outside of the normal
	 * PlayerControllerChanged flow (this is the equivalent of the old, subsystem-specific
	 * "StartUITree()" helper that used to live in ZWUIStateTreeSubsystem).
	 */
	UFUNCTION(BlueprintCallable, Category = "ZW|StateTree")
	void StartStateTree();

protected:
	/**
	 * Return the State Tree asset that this subsystem should run. Typically implemented by
	 * reading a TSoftObjectPtr<UStateTree> off your plugin's own Developer Settings and loading
	 * it synchronously. Called once, from Initialize().
	 */
	virtual const UStateTree* GetStateTreeAsset() const PURE_VIRTUAL(UZWStateTreeSubsystemBase::GetStateTreeAsset, return nullptr; );

	/**
	 * Bind your schema's external data descriptors (subsystem instance, player controller,
	 * pawn, etc.) onto the given execution context. Called before every Start/Tick/SendEvent.
	 */
	virtual void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset) PURE_VIRTUAL(UZWStateTreeSubsystemBase::BindContextData, );

	/** Sends a GameplayTag event into the currently running tree, if one is active. */
	void SendStateTreeEvent(FGameplayTag EventTag);

	/** Reference to the State Tree asset this subsystem drives. */
	UPROPERTY(Transient)
	FStateTreeReference StateTreeRef;

	/** Working memory (instance data) for the running tree. */
	UPROPERTY(Transient)
	FStateTreeInstanceData StateTreeInstanceData;
};

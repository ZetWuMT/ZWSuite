// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeInstanceData.h"
#include "StateTreeReference.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ZWUIStateTreeSubsystem.generated.h"

/**
 * Subsystem responsible for managing and executing the UI-specific State Tree.
 * It acts as the brain for UI flow, completely decoupled from Input logic.
 */
UCLASS()
class ZWUISTATETREE_API UZWUIStateTreeSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
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

	/** * Starts the execution of the UI State Tree. 
	 * Call this from the Player Controller when the UI is ready to be managed.
	 */
	void StartUITree();

	/** * Injects an event tag into the UI State Tree queue.
	 * @param UITag The gameplay tag representing the UI event (e.g., "UI.Event.Inventory.Closed")
	 */
	UFUNCTION(BlueprintCallable)
	void ProcessUITag(FGameplayTag UITag);
	
private:
	/** Helper function to inject external data required by the schema into the context. */
	void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset);

	/** Reference to the actual State Tree asset (should be assigned via Developer Settings). */
	UPROPERTY(Transient)
	FStateTreeReference StateTreeRef;

	/** Internal memory storage for the state machine execution. */
	UPROPERTY(Transient)
	FStateTreeInstanceData StateTreeInstanceData;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWStateTreeSubsystemBase.h"
#include "ZWInputSubsystem.generated.h"

struct FInputActionValue;
class UInputMappingContext;

// THIS IS A TEMPORARY SOLUTION. IDEALLY WE'LL ROUTE IT THROUGH UI STATE TREE
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputTagDelegate, FGameplayTag, SignalTag);

/**
 * Runs the ZW Input State Tree for the local player.
 *
 * All of the generic "own a tree, tick it, (re)start it when we get a PlayerController"
 * plumbing now lives in UZWStateTreeSubsystemBase (ZWStateTreeCore plugin). This class only
 * adds what is specific to the Input schema: reading the tree asset from
 * UZWInputStateTreeSettings, binding ZWInputSubsystem/PlayerController/Pawn context data, and
 * the Enhanced-Input-specific mapping-context helpers.
 */
UCLASS()
class ZWINPUTSTATETREE_API UZWInputSubsystem : public UZWStateTreeSubsystemBase
{
	GENERATED_BODY()
	
public:
	//~ Begin ULocalPlayerSubsystem Interface
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;
	//~ End ULocalPlayerSubsystem Interface

	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void ProcessInputTag(FGameplayTag InputTag, const FInputActionValue& InputActionValue);
	
	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void PushInputContext(const UInputMappingContext* IMC, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void PopInputContext(const UInputMappingContext* IMC);
	
	UPROPERTY(BlueprintAssignable, Category = "ZW|Input")
	FOnInputTagDelegate OnInputTagDelegate;

protected:
	//~ Begin UZWStateTreeSubsystemBase Interface
	virtual const UStateTree* GetStateTreeAsset() const override;
	virtual void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset) override;
	//~ End UZWStateTreeSubsystemBase Interface

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UInputMappingContext>> ActiveContexts;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeInstanceData.h"
#include "StateTreeReference.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "ZWInputSubsystem.generated.h"

struct FInputActionValue;
class UInputMappingContext;

// THIS IS A TEMPORARY SOLUTION. IDEALLY WE'LL ROUTE IT THROUGH UI STATE TREE
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnInputTagDelegate, FGameplayTag, SignalTag);

/**
 * 
 */
UCLASS()
class ZWINPUTSTATETREE_API UZWInputSubsystem : public ULocalPlayerSubsystem, public FTickableGameObject
{
	GENERATED_BODY()
	
public:
	
	// Funkcja do inicjalizacji i "tykania" drzewa
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	
	virtual void PlayerControllerChanged(APlayerController* NewPlayerController) override;

	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override;
	
	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void ProcessInputTag(FGameplayTag InputTag, const FInputActionValue& InputActionValue);
	
	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void PushInputContext(const UInputMappingContext* IMC, int32 Priority);

	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void PopInputContext(const UInputMappingContext* IMC);
	
	UPROPERTY(BlueprintAssignable, Category = "ZW|Input")
	FOnInputTagDelegate OnInputTagDelegate;
	
protected:
	/** Referencja do assetu drzewa, którą ustawisz np. w Initialize */
	UPROPERTY(EditAnywhere, Category = "Input")
	FStateTreeReference StateTreeRef;

	/** Dane instancji (pamięć robocza drzewa) */
	UPROPERTY(Transient)
	FStateTreeInstanceData StateTreeInstanceData;

private:
	void BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset);
	
	// Trzymamy listę tego, co sami dodaliśmy, żeby móc to wyczyścić np. przy restarcie
	UPROPERTY(Transient)
	TArray<TObjectPtr<const UInputMappingContext>> ActiveContexts;
};

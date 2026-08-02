// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "ZWActionManagerComponent.generated.h"

class UZWGameplayAction;
struct FInputActionValue;

UCLASS(ClassGroup=(ZW), meta=(BlueprintSpawnableComponent))
class ZWGAMEPLAYACTIONS_API UZWActionManagerComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UZWActionManagerComponent();

	// Granting actions (used by the InputStateTree Tasks!)
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void GrantAction(TSubclassOf<UZWGameplayAction> ActionClass);

	// Removing actions
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass);

	// Reacting to Input (bound to the Broadcast from ZWInputComponent)
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue);

protected:
	virtual void BeginPlay() override;

	// Actions the player has from the start (e.g. Jump, Walk)
	UPROPERTY(EditDefaultsOnly, Category = "ZW Actions")
	TArray<TSubclassOf<UZWGameplayAction>> DefaultActions;

	// Instances of currently owned actions
	UPROPERTY(Transient)
	TArray<TObjectPtr<UZWGameplayAction>> GrantedActions;
};

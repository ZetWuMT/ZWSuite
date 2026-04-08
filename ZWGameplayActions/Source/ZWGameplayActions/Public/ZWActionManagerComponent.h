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

	// Dodawanie akcji (używane przez InputStateTree Taski!)
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void GrantAction(TSubclassOf<UZWGameplayAction> ActionClass);

	// Odbieranie akcji
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void RemoveAction(TSubclassOf<UZWGameplayAction> ActionClass);

	// Reagowanie na Input (Podpinane pod Broadcast z ZWInputComponent)
	UFUNCTION(BlueprintCallable, Category = "ZW Actions")
	void HandleInputTag(FGameplayTag InputTag, const FInputActionValue& ActionValue);

protected:
	virtual void BeginPlay() override;

	// Akcje, które gracz ma od początku (np. Skok, Chodzenie)
	UPROPERTY(EditDefaultsOnly, Category = "ZW Actions")
	TArray<TSubclassOf<UZWGameplayAction>> DefaultActions;

	// Instancje aktualnie posiadanych akcji
	UPROPERTY(Transient)
	TArray<TObjectPtr<UZWGameplayAction>> GrantedActions;
};

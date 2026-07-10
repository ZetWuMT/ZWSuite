// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "ZWAbilitySystemComponent.generated.h"

class UZWAbilitiesConfig;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZWGASEXTENSIONS_API UZWAbilitySystemComponent : public UAbilitySystemComponent
{
	GENERATED_BODY()
	
public:
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;
	
	virtual void OnPlayerControllerSet() override;
	
	UFUNCTION(BlueprintCallable, Category = "ZWInput")
	void HandleInputTag(FGameplayTag InGameplayTag);
};

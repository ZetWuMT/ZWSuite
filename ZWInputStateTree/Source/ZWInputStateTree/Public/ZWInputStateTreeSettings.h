// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWInputSettings.h"
#include "ZWInputStateTreeSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Input"))
class ZWINPUTSTATETREE_API UZWInputStateTreeSettings : public UZWInputSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "StateTree", meta=(AllowedClasses="/Script/StateTreeModule.StateTree"))
	TSoftObjectPtr<UStateTree> DefaultInputStateTree;
};

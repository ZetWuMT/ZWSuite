// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZWUIStateTreeSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZWUI StateTree Settings"))
class ZWUISTATETREE_API UZWUIStateTreeSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category = "StateTree", meta=(AllowedClasses="/Script/StateTreeModule.StateTree"))
	TSoftObjectPtr<UStateTree> DefaultUIStateTree;
};

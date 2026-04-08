// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ZWInputSettings.generated.h"

class UZWInputConfig;
class UStateTree;
class UZWInputConfig_Old;

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Input"))
class ZWINPUT_API UZWInputSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UPROPERTY(Config, EditAnywhere, Category= "Input", meta=(ForceInlineRow))
	TSoftObjectPtr<UZWInputConfig> InputConfig;
	
#if WITH_EDITORONLY_DATA
	virtual FName GetCategoryName() const override { return FName("ZW"); }
	virtual FText GetSectionText() const override { return INVTEXT("ZW Input Settings"); }
#endif
};

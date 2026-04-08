// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ZWInteractionSystemSettings.generated.h"

/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Interaction System Settings"))
class ZWINTERACTION_API UZWInteractionSystemSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Interaction System", DisplayName="Interaction Collision Channel")
	TEnumAsByte<ECollisionChannel> InteractionCollisionChannel;
	
#if WITH_EDITORONLY_DATA
	virtual FName GetCategoryName() const override { return FName("ZW"); }
	virtual FText GetSectionText() const override { return INVTEXT("ZW Interaction Settings"); }
#endif
};

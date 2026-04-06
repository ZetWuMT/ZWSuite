// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "InputMappingContext.h"
#include "UObject/Object.h"
#include "ZWInteractionSystem_Settings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Interaction System Settings"))
class ZWINTERACTION_API UZWInteractionSystem_Settings : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(Config, EditAnywhere, Category = "Interaction System", DisplayName="Interaction Collision Channel")
	TEnumAsByte<ECollisionChannel> InteractionCollisionChannel;
};

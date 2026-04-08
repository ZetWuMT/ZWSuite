// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZWUIHUDElement.generated.h"

/**
 * 
 */
UCLASS()
class ZWUIHUD_API UZWUIHUDElement : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category = "HUD Slot")
	void InitializeHUDElement(AActor* OwnerActor);
};

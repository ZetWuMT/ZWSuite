// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ZWUIRootLayout.generated.h"

enum class EZWWidgetLayer : uint8;
class UCommonActivatableWidgetStack;
class UOverlay;
/**
 * 
 */
UCLASS(Abstract)
class ZWUICORE_API UZWUIRootLayout : public UUserWidget
{
	GENERATED_BODY()
	
public:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UOverlay> GameLayer;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> MenuLayer;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UCommonActivatableWidgetStack> PromptLayer;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonButtonBase.h"
#include "ZWUITabListButton.generated.h"

class UCommonTextBlock;
class UImage;
/**
 * 
 */
UCLASS(Abstract, BlueprintType)
class ZWUICORE_API UZWUITabListButton : public UCommonButtonBase
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable)
	void SetTabData(FText ButtonText, UTexture2D* ButtonIcon);
	
protected:		
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UImage> TabButtonImage;
	
	UPROPERTY(BlueprintReadOnly, meta = (BindWidgetOptional))
	TObjectPtr<UCommonTextBlock> TabButtonText;
};

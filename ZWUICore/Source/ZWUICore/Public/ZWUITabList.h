// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonTabListWidgetBase.h"
#include "ZWUITabList.generated.h"

class UPanelWidget;

/**
 * 
 */
UCLASS(Abstract, BlueprintType)
class ZWUICORE_API UZWUITabList : public UCommonTabListWidgetBase
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UPanelWidget> TabButtonContainer;
	
	virtual void HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
	virtual void HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton) override;
};

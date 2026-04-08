// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWUIPanel.h"
#include "ZWUIHUDPanel.generated.h"

class UZWUIRootLayout;
class UZWUIPlayerHUBWidget;
/**
 * 
 */
UCLASS(Abstract)
class ZWUIHUD_API UZWUIHUDPanel : public UZWUIPanel
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
};

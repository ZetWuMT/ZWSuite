// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "CommonActivatableWidget.h"
#include "GameplayTagContainer.h"
#include "ZWUIPanelDatabase.h"
#include "ZWUIPanel.generated.h"

class UInputAction;
/**
 * 
 */
UCLASS(Abstract)
class ZWUICORE_API UZWUIPanel : public UCommonActivatableWidget
{
	GENERATED_BODY()

public:
	UZWUIPanel(const FObjectInitializer& ObjectInitializer);
	
	//~ Begin UCommonActivatableWidget Interface
	virtual TOptional<FUIInputConfig> GetDesiredInputConfig() const override;
	//~ Begin UCommonActivatableWidget Interface
	
	// The tag that was used to summon this panel. Injected by the HUB upon creation.
	UPROPERTY(BlueprintReadOnly, Category = "ZW|UI")
	FGameplayTag PanelIdentityTag;
	
	// By default, it is set to true, as most of the panels require it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZW|UI")
	bool bRequiresBackground = true;
	
	UPROPERTY(Transient)
	FGameplayTag BoundPanelTag;

protected:
	//~ Begin UUserWidget Interface
	virtual void NativeOnInitialized() override;
	//~ End UUserWidget Interface
	
	//~ Begin UCommonActivatableWidget Interface
	virtual void NativeOnActivated() override;
	virtual void NativeOnDeactivated() override;
	virtual UWidget* NativeGetDesiredFocusTarget() const override;
	virtual bool NativeOnHandleBackAction() override;
	//~ End UCommonActivatableWidget Interface 
	
	// By default, it is set to true, as most of the panels require it.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ZW|UI")
	bool bRequiresInput = true;

private:
	/** Default behavior for closing a panel */
	//void HandleBackAction();
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DeveloperSettings.h"
#include "ZWInventorySettings.generated.h"

/**
 * Global settings for the ZWInventory plugin.
 * Accessible in Project Settings -> Plugins -> ZWInventory.
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="Inventory Settings"))
class ZWINVENTORY_API UZWInventorySettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UZWInventorySettings();
	/**
	 * If true, the inventory items can be stacked.
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Inventory", DisplayName="Enable Stacking")
	bool bEnableStacking;

	/**
	 * This is the tag used to specify, how many items there is stacked in a particular stack of this item.
	 * NOTE: It is NOT the amount of items in a pickupable stack but a quality of the stack existing in the inventory!
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Inventory", DisplayName="Stack Count Tag", meta=(EditCondition="bEnableStacking", EditConditionHides))
	FGameplayTag StackCountTag;
	
	/**
	*  This is the tag used to specify, how many items can be stacked in a single stack. 
	*  Adding more items to the inventory will add additional stack containing remaining items.
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Inventory", DisplayName="Max Stack Count Tag", meta=(EditCondition="bEnableStacking", EditConditionHides))
	FGameplayTag MaxStackCountTag;
	
	/**
	*  This is the tag used to specify, how many stacks of this item overall the player can possess. 
	*  If reached, the player will pick up only as many items from the instance on the level and the rest will remain as was.  
	*/
	UPROPERTY(Config, EditAnywhere, Category = "Inventory", DisplayName="Total Stack Count Tag", meta=(EditCondition="bEnableStacking", EditConditionHides))
	FGameplayTag TotalStackCountTag;
	
#if WITH_EDITORONLY_DATA
	virtual FName GetCategoryName() const override { return FName("ZW"); }
	virtual FText GetSectionText() const override { return INVTEXT("ZW Inventory Settings"); }
#endif
};

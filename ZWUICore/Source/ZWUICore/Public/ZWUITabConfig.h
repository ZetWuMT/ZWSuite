#pragma once
#include "GameplayTagContainer.h"

#include "ZWUITabConfig.generated.h"

class UInputAction;
class UCommonButtonBase;
class UCommonActivatableWidget;

USTRUCT(BlueprintType)
struct FZWUITabDefinition
{
	GENERATED_BODY()
	
	// Unique tab ID (e.g. "UI.Panel.Inventory")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	FGameplayTag TabTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab|State Tree")
	FGameplayTag StateTag;

	// Which widget should open in the Switcher
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	TSubclassOf<UCommonActivatableWidget> PanelClass;

	// How the button on the top bar should look
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	TSubclassOf<UCommonButtonBase> TabButtonClass;

	// (Optional) Icon and text for the button, if your button supports it
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	TObjectPtr<UTexture2D> TabIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	FText TabDisplayName;
};

UCLASS(BlueprintType)
class ZWUICORE_API UZWUITabConfig : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tabs")
	TArray<FZWUITabDefinition> Tabs;
};

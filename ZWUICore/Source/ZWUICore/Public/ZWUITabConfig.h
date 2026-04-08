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
	
	// Unikalne ID zakładki (np. "UI.Panel.Inventory")
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	FGameplayTag TabTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab|State Tree")
	FGameplayTag StateTag;

	// Jaki widget ma się otworzyć w Switcherze
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	TSubclassOf<UCommonActivatableWidget> PanelClass;

	// Jak ma wyglądać przycisk na górnym pasku
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tab")
	TSubclassOf<UCommonButtonBase> TabButtonClass;

	// (Opcjonalnie) Ikona i tekst do przycisku, jeśli Twój przycisk to obsługuje
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

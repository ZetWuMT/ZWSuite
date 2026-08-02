#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "ZWAssetDefinition_GameplayAction.generated.h"

UCLASS()
class ZWGAMEPLAYACTIONSEDITOR_API UZWAssetDefinition_GameplayAction : public UAssetDefinition
{
	GENERATED_BODY()

public:
	// 1. Name that is displayed when you hover over the asset
	virtual FText GetAssetDisplayName() const override;

	// 2. ASSET COLOR!
	virtual FLinearColor GetAssetColor() const override;

	// 3. Which C++ class does it refer to?
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	// 4. In which category in the menu (right click) should it appear?
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};
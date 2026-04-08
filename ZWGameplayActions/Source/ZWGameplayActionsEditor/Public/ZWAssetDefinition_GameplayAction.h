#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "ZWAssetDefinition_GameplayAction.generated.h"

UCLASS()
class ZWGAMEPLAYACTIONSEDITOR_API UZWAssetDefinition_GameplayAction : public UAssetDefinition
{
	GENERATED_BODY()

public:
	// 1. Nazwa, która wyświetla się, gdy najedziesz myszką na asset
	virtual FText GetAssetDisplayName() const override;

	// 2. KOLOR ASSETU!
	virtual FLinearColor GetAssetColor() const override;

	// 3. Do jakiej klasy C++ się to odnosi?
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;

	// 4. W jakiej kategorii w menu (prawy klik) ma się to pojawiać?
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};
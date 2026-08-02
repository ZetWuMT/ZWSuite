#include "ZWAssetDefinition_GameplayAction.h"
#include "ZWGameplayAction.h" // Your base header from the Runtime module!

FText UZWAssetDefinition_GameplayAction::GetAssetDisplayName() const
{
	return NSLOCTEXT("AssetDefinition", "AssetDefinition_ZWGameplayAction", "ZW Gameplay Action");
}

FLinearColor UZWAssetDefinition_GameplayAction::GetAssetColor() const
{
	// Here you define the color (R, G, B, Alpha). 
	// Values from 0.0 to 1.0. 
	// I am setting a nice, aggressive red (e.g. R: 0.8, G: 0.1, B: 0.1)
	return FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
}

TSoftClassPtr<UObject> UZWAssetDefinition_GameplayAction::GetAssetClass() const
{
	// Important: we return our base class. UE5 is smart enough
	// to automatically color all Blueprints deriving from this class!
	return UZWGameplayAction::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UZWAssetDefinition_GameplayAction::GetAssetCategories() const
{
	// Instead of pushing it into the generic "Blueprints", let us create our own category!
	static const auto Categories = { FAssetCategoryPath(NSLOCTEXT("AssetDefinition", "ZWFrameworkCategory", "ZW Framework")) };
	return Categories;
}
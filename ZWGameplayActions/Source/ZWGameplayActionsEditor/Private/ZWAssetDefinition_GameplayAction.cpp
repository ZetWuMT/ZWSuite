#include "ZWAssetDefinition_GameplayAction.h"
#include "ZWGameplayAction.h" // Twój bazowy nagłówek z modułu Runtime!

FText UZWAssetDefinition_GameplayAction::GetAssetDisplayName() const
{
	return NSLOCTEXT("AssetDefinition", "AssetDefinition_ZWGameplayAction", "ZW Gameplay Action");
}

FLinearColor UZWAssetDefinition_GameplayAction::GetAssetColor() const
{
	// Tutaj definiujesz kolor (R, G, B, Alpha). 
	// Wartości od 0.0 do 1.0. 
	// Ustawiam na ładny, agresywny czerwony (np. R: 0.8, G: 0.1, B: 0.1)
	return FLinearColor(0.8f, 0.1f, 0.1f, 1.0f);
}

TSoftClassPtr<UObject> UZWAssetDefinition_GameplayAction::GetAssetClass() const
{
	// Ważne: Zwracamy naszą klasę bazową. UE5 jest na tyle mądre, 
	// że automatycznie pokoloruje wszystkie Blueprinty dziedziczące po tej klasie!
	return UZWGameplayAction::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UZWAssetDefinition_GameplayAction::GetAssetCategories() const
{
	// Zamiast pchać to do ogólnego "Blueprints", stwórzmy własną kategorię!
	static const auto Categories = { FAssetCategoryPath(NSLOCTEXT("AssetDefinition", "ZWFrameworkCategory", "ZW Framework")) };
	return Categories;
}
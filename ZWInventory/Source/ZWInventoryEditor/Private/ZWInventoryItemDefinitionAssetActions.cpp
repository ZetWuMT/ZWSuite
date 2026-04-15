// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryItemDefinitionAssetActions.h"
#include "ZWInventoryEditor.h"
#include "ZWInventoryItemDefinition.h"

FText ZWInventoryItemDefinitionAssetActions::GetName() const
{
	return NSLOCTEXT("AssetTypeActions", "AssetTypeActions_InventoryItemDefinition", "Inventory Item Definition");
}

UClass* ZWInventoryItemDefinitionAssetActions::GetSupportedClass() const
{
	return UZWInventoryItemDefinition::StaticClass();
}

FColor ZWInventoryItemDefinitionAssetActions::GetTypeColor() const
{
	return FColor::Cyan;
}

uint32 ZWInventoryItemDefinitionAssetActions::GetCategories()
{
	return FModuleManager::LoadModuleChecked<FZWInventoryEditorModule>("ZWInventoryEditor").GetInventoryAssetCategory();
}

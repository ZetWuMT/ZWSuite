#include "ZWInventoryEditor.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "ZWInventoryItemDefinitionAssetActions.h"

#define LOCTEXT_NAMESPACE "FZWInventoryEditorModule"

void FZWInventoryEditorModule::StartupModule()
{
	IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();

	Inventory_AssetCategory = AssetTools.RegisterAdvancedAssetCategory(
		FName(TEXT("Inventory")), 
		FText::FromString("Inventory") 
	);
	
	AssetTools.RegisterAssetTypeActions(MakeShareable(new ZWInventoryItemDefinitionAssetActions));
}

void FZWInventoryEditorModule::ShutdownModule()
{
    
}

#undef LOCTEXT_NAMESPACE
    
IMPLEMENT_MODULE(FZWInventoryEditorModule, ZWInventoryEditor)
#pragma once

#include "CoreMinimal.h"
#include "AssetTypeCategories.h"
#include "Modules/ModuleManager.h"

class FZWInventoryEditorModule : public IModuleInterface
{
public:
    virtual void StartupModule() override;
    virtual void ShutdownModule() override;
    
    EAssetTypeCategories::Type GetInventoryAssetCategory() const { return Inventory_AssetCategory; }
    
    private:
    EAssetTypeCategories::Type Inventory_AssetCategory;
};

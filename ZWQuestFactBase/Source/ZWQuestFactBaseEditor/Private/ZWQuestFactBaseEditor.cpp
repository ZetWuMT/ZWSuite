// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWQuestFactBaseEditor.h"
#include "ZWQuestFactBaseEditorStyle.h"
#include "ZWQuestFactBaseEditorCommands.h"
#include "SZWQuestFactBaseEditor.h"
#include "ZWQuestFactNameCustomization.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName QuestFactBaseEditorTabName("Quest Fact Base Editor");

#define LOCTEXT_NAMESPACE "FZWQuestFactBaseEditorModule"

void FZWQuestFactBaseEditorModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module

	
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	
	// Rejestrujemy naszą customizację
	PropertyModule.RegisterCustomPropertyTypeLayout(
		FZWQuestFactSearchableName::StaticStruct()->GetFName(),
		FOnGetPropertyTypeCustomizationInstance::CreateStatic(&FZWQuestFactNameCustomization::MakeInstance)
	);
	
	FZWQuestFactBaseEditorStyle::Initialize();
	FZWQuestFactBaseEditorStyle::ReloadTextures();

	FZWQuestFactBaseEditorCommands::Register();
	
	PluginCommands = MakeShareable(new FUICommandList);

	PluginCommands->MapAction(
		FZWQuestFactBaseEditorCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FZWQuestFactBaseEditorModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FZWQuestFactBaseEditorModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(QuestFactBaseEditorTabName, FOnSpawnTab::CreateRaw(this, &FZWQuestFactBaseEditorModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FQuestFactBaseTabTitle", "QuestFactBaseEditor"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FZWQuestFactBaseEditorModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FZWQuestFactBaseEditorStyle::Shutdown();

	FZWQuestFactBaseEditorCommands::Unregister();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(QuestFactBaseEditorTabName);

	if (FModuleManager::Get().IsModuleLoaded("PropertyEditor"))
	{
		FPropertyEditorModule& PropertyModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyModule.UnregisterCustomPropertyTypeLayout(FZWQuestFactSearchableName::StaticStruct()->GetFName());
	}
}

TSharedRef<SDockTab> FZWQuestFactBaseEditorModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SZWQuestFactBaseEditor)
		];
}

void FZWQuestFactBaseEditorModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(QuestFactBaseEditorTabName);
}

void FZWQuestFactBaseEditorModule::RegisterMenus()
{
	// Owner will be used for cleanup in call to UToolMenus::UnregisterOwner
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		{
			FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
			Section.AddMenuEntryWithCommandList(FZWQuestFactBaseEditorCommands::Get().OpenPluginWindow, PluginCommands);
		}
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar");
		{
			FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("Settings");
			{
				FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FZWQuestFactBaseEditorCommands::Get().OpenPluginWindow));
				Entry.SetCommandList(PluginCommands);
			}
		}
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZWQuestFactBaseEditorModule, ZWQuestFactBaseEditor)
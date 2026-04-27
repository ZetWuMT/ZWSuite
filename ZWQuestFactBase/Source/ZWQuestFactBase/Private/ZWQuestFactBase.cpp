// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWQuestFactBase.h"
#include "ZWQuestFactBaseStyle.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Text/STextBlock.h"
#include "ToolMenus.h"

static const FName QuestFactBaseTabName("ZWQuestFactBase");

#define LOCTEXT_NAMESPACE "FZWQuestFactBaseModule"

void FZWQuestFactBaseModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FZWQuestFactBaseStyle::Initialize();
	FZWQuestFactBaseStyle::ReloadTextures();

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FZWQuestFactBaseModule::RegisterMenus));
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(QuestFactBaseTabName, FOnSpawnTab::CreateRaw(this, &FZWQuestFactBaseModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("FQuestFactBaseTabTitle", "QuestFactBase"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);
}

void FZWQuestFactBaseModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	UToolMenus::UnRegisterStartupCallback(this);

	UToolMenus::UnregisterOwner(this);

	FZWQuestFactBaseStyle::Shutdown();

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(QuestFactBaseTabName);
}

TSharedRef<SDockTab> FZWQuestFactBaseModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SButton)
		];
}

void FZWQuestFactBaseModule::PluginButtonClicked()
{
	FGlobalTabmanager::Get()->TryInvokeTab(QuestFactBaseTabName);
}

void FZWQuestFactBaseModule::RegisterMenus()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZWQuestFactBaseModule, ZWQuestFactBase)
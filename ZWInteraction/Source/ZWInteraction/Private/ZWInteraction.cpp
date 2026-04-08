// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWInteraction.h"

#include "ISettingsModule.h"
#include "ZWInteractionSystem_Settings.h"

#define LOCTEXT_NAMESPACE "FZWInteractionModule"

void FZWInteractionModule::StartupModule()
{
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->RegisterSettings("Project", "ProjectX", "InteractionSystem_Settings",
			LOCTEXT("RuntimeSettingsName", "Interaction System Settings"), LOCTEXT("RuntimeSettingsDescription", "Configure the Interaction System"),
			GetMutableDefault<UZWInteractionSystem_Settings>());
	}
}

void FZWInteractionModule::ShutdownModule()
{
	if(ISettingsModule* SettingsModule = FModuleManager::GetModulePtr<ISettingsModule>("Settings"))
	{
		SettingsModule->UnregisterSettings("Project", "ProjectX", "InteractionSystem_Settings");
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZWInteractionModule, ZWInteraction)
// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWUIInputBridgeModule.h"

#include "ZWInputSettings.h"
#include "ZWUISettings.h"
#include "Misc/CoreDelegates.h"

#if WITH_EDITOR
#include "Engine/DeveloperSettings.h"
#endif

#define LOCTEXT_NAMESPACE "FZWUIInputBridgeModule"

void FZWUIInputBridgeModule::StartupModule()
{
	// Run once engine init has fully completed - this is safe to call in both the editor and
	// packaged games, and guarantees both settings CDOs are fully loaded from their config
	// files before we touch them.
	FCoreDelegates::GetOnPostEngineInit().AddStatic(&FZWUIInputBridgeModule::SyncInputConfig);

#if WITH_EDITOR
	// Best-effort live sync: if a designer edits ZWInput's InputConfig in Project Settings
	// while the editor is running, mirror the change into ZWUICore's copy immediately instead
	// of waiting for an editor restart.
	//
	// NOTE: UDeveloperSettings::OnSettingChanged() is available on UE5; if your specific engine
	// version doesn't expose it, simply delete this WITH_EDITOR block. The OnPostEngineInit
	// sync above is still enough to cover editor/game startup - it just won't live-update
	// mid-session in that case.
	//SettingsChangedHandle = UDeveloperSettings::OnSettingChanged().AddStatic(&FZWUIInputBridgeModule::OnAnySettingChanged);
#endif
}

void FZWUIInputBridgeModule::ShutdownModule()
{
#if WITH_EDITOR
	//UDeveloperSettings::OnSettingChanged().Remove(SettingsChangedHandle);
#endif
}

void FZWUIInputBridgeModule::SyncInputConfig()
{
	const UZWInputSettings* InputSettings = GetDefault<UZWInputSettings>();
	UZWUISettings* UISettings = GetMutableDefault<UZWUISettings>();

	if (InputSettings && UISettings)
	{
		// ZWInput is the single source of truth for which UZWInputConfig is active whenever
		// this bridge is enabled. This only changes the in-memory CDO value - it does not call
		// SaveConfig()/UpdateDefaultConfigFile(), so it never gets written back into
		// DefaultZWUICore.ini. If the bridge is later disabled, ZWUICore's config ini is
		// untouched and the plugin reverts to reading whatever was last saved there.
		UISettings->InputConfig = InputSettings->InputConfig;
	}
}

#if WITH_EDITOR
void FZWUIInputBridgeModule::OnAnySettingChanged(UObject* Settings)
{
	if (Settings && Settings->IsA<UZWInputSettings>())
	{
		SyncInputConfig();
	}
}
#endif

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FZWUIInputBridgeModule, ZWUIInputBridge)

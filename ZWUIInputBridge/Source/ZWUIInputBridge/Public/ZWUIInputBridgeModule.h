// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * This module has one job: make sure ZWUICore's UZWInputConfig reference (UZWUISettings::
 * InputConfig) always mirrors ZWInput's (UZWInputSettings::InputConfig), so a project using
 * both plugins has a single source of truth instead of two independently editable copies.
 *
 * Neither ZWInput nor ZWUICore know this module exists - they are never modified by it beyond
 * having their existing, public UPROPERTY read/written through their existing public API.
 * If this plugin is disabled, both settings objects behave exactly as if this module never
 * existed (each keeps its own independently configured InputConfig).
 */
class FZWUIInputBridgeModule : public IModuleInterface
{
public:
	//~ Begin IModuleInterface Interface
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	//~ End IModuleInterface Interface

private:
	/** Copies UZWInputSettings::InputConfig into UZWUISettings::InputConfig (in memory only - never writes to config ini). */
	static void SyncInputConfig();

#if WITH_EDITOR
	/** Reacts to any UDeveloperSettings CDO being edited in Project Settings, re-syncing if it was UZWInputSettings. */
	static void OnAnySettingChanged(UObject* Settings);

	FDelegateHandle SettingsChangedHandle;
#endif
};

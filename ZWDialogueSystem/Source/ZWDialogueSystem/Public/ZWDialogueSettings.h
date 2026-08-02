// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueSystemAudioGenerationData.h"
#include "Engine/DeveloperSettings.h"
#include "ZWDialogueSettings.generated.h"
/**
 * 
 */
UCLASS(Config=Game, defaultconfig, meta=(DisplayName="ZW Dialogue System"))
class ZWDIALOGUESYSTEM_API UZWDialogueSettings : public UDeveloperSettings
{
	GENERATED_BODY()
	
public:
	UZWDialogueSettings();

	// API key for Google Cloud TTS
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString TTSGeneratorApiKey;

	// Main folder for the generated WAV files (e.g. "Localization/Audio")
	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FDirectoryPath BaseAudioExportPath;

	// Default developer language (e.g. "db-db" or "pl-PL")
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString DefaultLanguageCode;
    
	// Default voice (e.g. "pl-PL-Wavenet-B")
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString DefaultVoiceName;
	
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	TSoftObjectPtr<UZWDialogueSystemAudioGenerationData> AudioGenerationData;
};

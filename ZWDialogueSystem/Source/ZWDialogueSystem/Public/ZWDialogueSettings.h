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

	// Klucz API do Google Cloud TTS
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString TTSGeneratorApiKey;

	// Główny folder dla wygenerowanych plików WAV (np. "Localization/Audio")
	UPROPERTY(Config, EditAnywhere, Category = "Paths")
	FDirectoryPath BaseAudioExportPath;

	// Domyślny język deweloperski (np. "db-db" lub "pl-PL")
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString DefaultLanguageCode;
    
	// Domyślny głos (np. "pl-PL-Wavenet-B")
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	FString DefaultVoiceName;
	
	UPROPERTY(Config, EditAnywhere, Category = "TTS")
	TSoftObjectPtr<UZWDialogueSystemAudioGenerationData> AudioGenerationData;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "ZWDialogueSystemAudioGenerationData.generated.h"

/**
 * 
 */
UCLASS()
class ZWDIALOGUESYSTEM_API UZWDialogueSystemAudioGenerationData : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, Category = "TTS")
	TMap<FName, FString> SpeakerVoiceNames;
};

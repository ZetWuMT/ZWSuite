#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueAudioData.generated.h"

// Represents the physical audio data
USTRUCT(BlueprintType)
struct FZWDialogueAudioData
{
	GENERATED_BODY()

	// Unique file identifier (will also be the .wav file name in the Localization folder)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Audio")
	FGuid AudioGuid;

	// Useful in Sequencer so the section knows in advance how long it is,
	// without asynchronously loading the file from disk in the editor
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Audio")
	float PrecalculatedDuration = 0.0f; 
};
#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueAudioData.generated.h"

// Reprezentuje fizyczne dane audio
USTRUCT(BlueprintType)
struct FZWDialogueAudioData
{
	GENERATED_BODY()

	// Unikalny identyfikator pliku (będzie też nazwą pliku .wav w folderze Localization)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Audio")
	FGuid AudioGuid;

	// Przydatne w Sequencerze, żeby sekcja wiedziała z góry, jak jest długa, 
	// bez asynchronicznego ładowania pliku z dysku w edytorze
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue|Audio")
	float PrecalculatedDuration = 0.0f; 
};
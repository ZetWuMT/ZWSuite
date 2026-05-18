#pragma once

#include "ZWDialogueAudioData.h"
#include "UObject/WeakInterfacePtr.h"

class IZWDialogueLineHandler;

USTRUCT(BlueprintType)
struct FZWDialogueData
{
	GENERATED_BODY()

public:
	FZWDialogueData();
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FGuid EventID;

	/**
	 * This is the FName identifier of the speaker. The localizable version should be defined separately.
	 * You need to define the GetAvailableSpeakers method to determine, who can be a speaker.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue", meta = (GetOptions = "GetAvailableSpeakers"))
	FName SpeakerID;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText Speaker;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Dialogue")
	FText DialogueLine;

	TWeakInterfacePtr<IZWDialogueLineHandler> FinalDialogueLineHandler = nullptr;
	
	FZWDialogueAudioData AudioData;
};


#pragma once

#include "RuntimeAudioImporterLibrary.h"
#include "ZWDialogueData.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZWMovieSceneDialogueSubsystem.generated.h"

class ULevelSequence;
class FZWDialogueToken;
class IZWDialogueLineHandler;

using FZWDialogueTokenPtr = TSharedPtr<FZWDialogueToken>;


//TODO: Potential multi-casts for additional subsystems if needed
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueStartedEvent, const FGuid);
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueEndedEvent, const FGuid);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDialogueTickEvent, const FGuid, float);


class FZWDialogueToken
{
public:
    FZWDialogueToken(UZWMovieSceneDialogueSubsystem* DialogueSubsystem, const FGuid& DialogueEventID);
    ~FZWDialogueToken();

public:
    FGuid EventID;

private:
    TWeakObjectPtr<UZWMovieSceneDialogueSubsystem> DialogueSubsystemWeak;
};

struct FZWDialogueDetails
{
    FName SpeakerID;
    FText Speaker;
    FText DialogueText;

    float Progress; // In Seconds
};



UCLASS()
class ZWDIALOGUESYSTEM_API UZWMovieSceneDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    
    //@TODO: Potential multi-casts for additional subsystems if needed
    FDialogueStartedEvent DialogueStartedEvent;
    FDialogueEndedEvent DialogueEndedEvent;
    FDialogueTickEvent DialogueTickEvent;
    
    virtual void Deinitialize() override;

    void TickDialogue(FGuid EventID, float CurrentTime);

    FZWDialogueTokenPtr TriggerDialogueLine(const FZWDialogueDetails& DialogueLine);
    FZWDialogueTokenPtr TriggerDialogueLine(FZWDialogueData DialogueData);

    void RegisterDialogueHandler(IZWDialogueLineHandler* Handler);
    void UnregisterDialogueHandler(IZWDialogueLineHandler* Handler);

private: 
    void CloseDialogueLine(const FGuid& EventID);
    
    void NotifyHandlers(FZWDialogueData DialogueData);
    
    void OnAudioImportProgress(int32 InProgress);
    void OnAudioImportFinished(URuntimeAudioImporterLibrary* Importer, UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Result);
    
    TArray<FZWDialogueData> CurrentDialogueLines;

    TArray<TWeakInterfacePtr<IZWDialogueLineHandler>> DialogueLineHandlers;

    friend FZWDialogueToken;
    
    // Map holding the active dialogue tokens tied to EventIDs
    TMap<FGuid, FZWDialogueData> PendingAudioDialogues;

    // Map linking a specific importer instance to an EventID so we know
    // which file just finished decoding on the thread
    UPROPERTY()
    TMap<FGuid, URuntimeAudioImporterLibrary*> ActiveImports;

    // Internal helper structure for tracking time while audio is still loading
    TMap<FGuid, float> PendingPlaybackTimes;
};
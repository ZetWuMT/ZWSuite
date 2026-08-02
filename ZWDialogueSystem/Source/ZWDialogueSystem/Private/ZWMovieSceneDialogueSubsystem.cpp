#include "ZWMovieSceneDialogueSubsystem.h"

#include "ZWDialogueData.h"
#include "ZWDialogueLineHandler.h"
#include "Components/AudioComponent.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
//#include "LevelSequence.h"



FZWDialogueToken::FZWDialogueToken(UZWMovieSceneDialogueSubsystem* DialogueSubsystem, const FGuid& DialogueEventID) 
    : EventID(DialogueEventID)
    , DialogueSubsystemWeak(DialogueSubsystem)
{
}

FZWDialogueToken::~FZWDialogueToken()
{
    if (UZWMovieSceneDialogueSubsystem* DialogueSubsystem = DialogueSubsystemWeak.Get())
    {
        DialogueSubsystem->CloseDialogueLine(EventID);
    }
}

void UZWMovieSceneDialogueSubsystem::Deinitialize()
{
    DialogueLineHandlers.Empty();
    /*
    TODO: Potential multi-casts for additional subsystems if needed
    DialogueStartedEvent.Clear();
    DialogueEndedEvent.Clear();
    */

    Super::Deinitialize();
}

void UZWMovieSceneDialogueSubsystem::TickDialogue(FGuid EventID, float CurrentTime)
{   
    if (CurrentDialogueLines.IsEmpty()) return;
    
    //TODO: Potential multi-casts for additional subsystems if needed
    DialogueTickEvent.Broadcast(EventID, CurrentTime);    
    
    const FZWDialogueData* CloseDialogueData = CurrentDialogueLines.FindByKey(EventID);
    check(CloseDialogueData != nullptr);
    
    for (const TWeakInterfacePtr<IZWDialogueLineHandler>& DialogueLineHandlerWeak : DialogueLineHandlers)
    {
        if (IZWDialogueLineHandler* DialogueLineHandler = DialogueLineHandlerWeak.Get())
        {
            DialogueLineHandler->OnDialogueLineUpdated(*CloseDialogueData);
            if (CloseDialogueData->FinalDialogueLineHandler == DialogueLineHandlerWeak)
            {
                break;
            }
        }
    }    
}

FZWDialogueTokenPtr UZWMovieSceneDialogueSubsystem::TriggerDialogueLine(const FZWDialogueDetails& DialogueDetails)
{
    const FGuid DialogueEventID = FGuid::NewGuid();

    FZWDialogueData DialogueData;
    DialogueData.EventID = DialogueEventID;
    DialogueData.SpeakerID = DialogueDetails.SpeakerID;
    DialogueData.Speaker = DialogueDetails.Speaker;
    DialogueData.DialogueLine = DialogueDetails.DialogueText;

    return TriggerDialogueLine(DialogueData);
}

FZWDialogueTokenPtr UZWMovieSceneDialogueSubsystem::TriggerDialogueLine(FZWDialogueData DialogueData)
{
    // Create and return a lightweight Token (Sequencer needs it immediately)
    FZWDialogueTokenPtr NewToken = MakeShared<FZWDialogueToken>(this, DialogueData.EventID);

    bool bIsAudioLoading = false;

    if (DialogueData.AudioData.AudioGuid.IsValid())
    {
        FString Lang = TEXT("en-gb"); 
        FString FullPath = FPaths::ProjectContentDir() / TEXT("Localization/Audio") / Lang / (DialogueData.AudioData.AudioGuid.ToString() + TEXT(".wav"));

        TArray<uint8> RawWavBytes;
        if (FFileHelper::LoadFileToArray(RawWavBytes, *FullPath) && RawWavBytes.Num() > 0)
        {
            URuntimeAudioImporterLibrary* Importer = URuntimeAudioImporterLibrary::CreateRuntimeAudioImporter();
            if (Importer)
            {
                ActiveImports.Add(DialogueData.EventID, Importer);
                PendingPlaybackTimes.Add(DialogueData.EventID, 0.0f); 
                
                // BACKUP COPY: Dump the data into the pending map so we can retrieve it in OnAudioImportFinished
                PendingAudioDialogues.Add(DialogueData.EventID, DialogueData);

                Importer->OnProgressNative.AddUObject(this, &UZWMovieSceneDialogueSubsystem::OnAudioImportProgress);
                Importer->OnResultNative.AddUObject(this, &UZWMovieSceneDialogueSubsystem::OnAudioImportFinished);
                Importer->ImportAudioFromFile(FullPath, ERuntimeAudioFormat::Auto);
                //Importer->ImportAudioFromBuffer(RawWavBytes, ERuntimeAudioFormat::Wav);
                
                bIsAudioLoading = true;
            }
        }
    }

    // If for some reason we are not loading audio (missing GUID or I/O error),
    // fire the Handlers right away so the UI is not blocked.
    if (!bIsAudioLoading)
    {
        NotifyHandlers(DialogueData);
    }

    return NewToken;
}

void UZWMovieSceneDialogueSubsystem::CloseDialogueLine(const FGuid& EventID)
{
    if (CurrentDialogueLines.IsEmpty()) return;
    const FZWDialogueData* CloseDialogueData = CurrentDialogueLines.FindByKey(EventID);
    check(CloseDialogueData != nullptr);
    
    for (const TWeakInterfacePtr<IZWDialogueLineHandler>& DialogueLineHandlerWeak : DialogueLineHandlers)
    {
        if (IZWDialogueLineHandler* DialogueLineHandler = DialogueLineHandlerWeak.Get())
        {
            DialogueLineHandler->OnFinishDialogueLine(*CloseDialogueData);
            if (CloseDialogueData->FinalDialogueLineHandler == DialogueLineHandlerWeak)
            {
                break;
            }
        }
    }

    const int32 Index = CurrentDialogueLines.IndexOfByKey(EventID);

    CurrentDialogueLines.RemoveAt(Index);
}

void UZWMovieSceneDialogueSubsystem::NotifyHandlers( FZWDialogueData DialogueData)
{
    for (const TWeakInterfacePtr<IZWDialogueLineHandler>& DialogueLineHandlerWeak : DialogueLineHandlers)
    {
        if (IZWDialogueLineHandler* DialogueLineHandler = DialogueLineHandlerWeak.Get())
        {
            const EZWStartDialogueResult Result = DialogueLineHandler->OnStartDialogueLine(DialogueData);

            if (Result == EZWStartDialogueResult::Final)
            {
                DialogueData.FinalDialogueLineHandler = DialogueLineHandlerWeak;
                break;
            }
        }
    }

    CurrentDialogueLines.Add(DialogueData);
}

void UZWMovieSceneDialogueSubsystem::OnAudioImportProgress(int32 InProgress)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioImportProgress: %d"), InProgress)
}

void UZWMovieSceneDialogueSubsystem::OnAudioImportFinished(URuntimeAudioImporterLibrary* Importer,
                                                           UImportedSoundWave* ImportedSoundWave, ERuntimeImportStatus Result)
{
    UE_LOG(LogTemp, Warning, TEXT("AudioImportFinished"))
    if (!Importer) return;

    // 1. Guard and extract the EventID
    FGuid EventID;
    if (const FGuid* FoundEventID = ActiveImports.FindKey(Importer))
    {
        EventID = *FoundEventID;
        //ActiveImports.Remove(Importer);
    }
    else return;

    // 2. Pull the suspended dialogue data out of the "waiting room"
    FZWDialogueData PendingData;
    if (FZWDialogueData* FoundData = PendingAudioDialogues.Find(EventID))
    {
        PendingData = *FoundData;
        PendingAudioDialogues.Remove(EventID);
    }
    else return; // No data, nothing to invoke in the UI

    // 3. Check success
    if (Result == ERuntimeImportStatus::SuccessfulImport && ImportedSoundWave)
    {        
        // Create the component (with bAutoDestroy = true so it does not clog RAM when done)
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(GetWorld(), ImportedSoundWave, 1.0f, 1.0f, 0.0f, nullptr, false, true);
        if (AudioComp)
        {
            // Play back the pending Sequencer time
            float TargetStartTime = 0.0f;
            if (float* PreTickTime = PendingPlaybackTimes.Find(EventID))
            {
                TargetStartTime = *PreTickTime;
            }

            AudioComp->Play(TargetStartTime);
            
            if (ActiveImports.Find(EventID))
            {
                ActiveImports.Remove(EventID);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Error while decoding on the thread! EventID: %s"), *EventID.ToString());
    }

    PendingPlaybackTimes.Remove(EventID);

    // 4. FIRE THE UI: audio started playing (or died), release the UI passing it the assembled structure
    NotifyHandlers(PendingData);
}

void UZWMovieSceneDialogueSubsystem::RegisterDialogueHandler(IZWDialogueLineHandler* Handler)
{
    DialogueLineHandlers.Add(Handler);
    DialogueLineHandlers.Sort([](const TWeakInterfacePtr<IZWDialogueLineHandler>& LhsWeak, const TWeakInterfacePtr<IZWDialogueLineHandler>& RhsWeak)
        {
            const IZWDialogueLineHandler* Lhs = LhsWeak.Get();
            const IZWDialogueLineHandler* Rhs = RhsWeak.Get();

            if (!Lhs) { return false; }
            if (!Rhs) { return true; }

            ensureMsgf(Lhs->GetOrder() != Rhs->GetOrder(), TEXT("Orders need to be unique"));
            return Lhs->GetOrder() < Rhs->GetOrder();
        });
}

void UZWMovieSceneDialogueSubsystem::UnregisterDialogueHandler(IZWDialogueLineHandler* Handler)
{
    DialogueLineHandlers.RemoveSingle(Handler);
}

bool operator== (FZWDialogueData DialogueData, FGuid EventID)
{
    return DialogueData.EventID == EventID;
}
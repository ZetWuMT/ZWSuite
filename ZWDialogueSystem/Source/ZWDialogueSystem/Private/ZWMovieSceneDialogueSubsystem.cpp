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
    // Tworzymy i zwracamy lekki Token (Sequencer go potrzebuje natychmiast)
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
                
                // KOPIA ZAPASOWA: Zrzucamy dane do mapy oczekującej, żeby wyjąć je w OnAudioImportFinished
                PendingAudioDialogues.Add(DialogueData.EventID, DialogueData);

                Importer->OnProgressNative.AddUObject(this, &UZWMovieSceneDialogueSubsystem::OnAudioImportProgress);
                Importer->OnResultNative.AddUObject(this, &UZWMovieSceneDialogueSubsystem::OnAudioImportFinished);
                Importer->ImportAudioFromFile(FullPath, ERuntimeAudioFormat::Auto);
                //Importer->ImportAudioFromBuffer(RawWavBytes, ERuntimeAudioFormat::Wav);
                
                bIsAudioLoading = true;
            }
        }
    }

    // Jeśli z jakiegoś powodu audio nie ładujemy (brak GUIDa lub błąd I/O), 
    // odpalamy Handlery od razu, żeby nie zablokować UI.
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

    // 1. Zabezpieczenie i wyciągnięcie EventID
    FGuid EventID;
    if (const FGuid* FoundEventID = ActiveImports.FindKey(Importer))
    {
        EventID = *FoundEventID;
        //ActiveImports.Remove(Importer);
    }
    else return;

    // 2. Wyciągamy zawieszone dane dialogowe z "poczekalni"
    FZWDialogueData PendingData;
    if (FZWDialogueData* FoundData = PendingAudioDialogues.Find(EventID))
    {
        PendingData = *FoundData;
        PendingAudioDialogues.Remove(EventID);
    }
    else return; // Brak danych, nie mamy czego wywołać w UI

    // 3. Sprawdzamy sukces
    if (Result == ERuntimeImportStatus::SuccessfulImport && ImportedSoundWave)
    {        
        // Tworzymy komponent (z bAutoDestroy = true, żeby nie zapychał RAMu po skończeniu)
        UAudioComponent* AudioComp = UGameplayStatics::SpawnSound2D(GetWorld(), ImportedSoundWave, 1.0f, 1.0f, 0.0f, nullptr, false, true);
        if (AudioComp)
        {
            // Odtwarzamy zaległy czas Sequencera
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
        UE_LOG(LogTemp, Error, TEXT("Błąd dekodowania w wątku! EventID: %s"), *EventID.ToString());
    }

    PendingPlaybackTimes.Remove(EventID);

    // 4. ODPALENIE UI: Audio zaczęło grać (lub padło na amen), uwalniamy UI podając mu zmontowaną strukturę
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
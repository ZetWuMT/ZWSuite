#include "ZWMovieSceneDialogueSubsystem.h"

#include "ZWDialogueData.h"
#include "ZWDialogueLineHandler.h"
#include "Engine/Engine.h"
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
    return MakeShared<FZWDialogueToken>(this, DialogueData.EventID);
}

void UZWMovieSceneDialogueSubsystem::CloseDialogueLine(const FGuid& EventID)
{
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
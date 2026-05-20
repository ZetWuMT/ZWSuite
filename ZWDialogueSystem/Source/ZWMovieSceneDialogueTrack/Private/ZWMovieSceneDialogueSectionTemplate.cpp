// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWMovieSceneDialogueSectionTemplate.h"
#include "Engine/GameInstance.h"
#include "Engine/World.h"
#include "Evaluation/MovieSceneEvaluation.h"
#include "IMovieScenePlayer.h"
#include "ZWDialogueData.h"
#include "ZWMovieSceneDialogueSubsystem.h"
#include "ZWMovieSceneDialogueSection.h"
#include "Engine/Engine.h"

#define LOCTEXT_NAMESPACE "ZWMovieSceneDialogueSectionTemplate"

struct FZWMovieSceneDialoguePersistentData : IPersistentEvaluationData
{
    FZWDialogueTokenPtr Token;
};

struct FZWMovieSceneDialogueEditorTemplatePersistentData : IPersistentEvaluationData
{
    FMovieSceneSequenceID SequenceID;
};

struct FZWMovieSceneDialogueExecutionToken final : IMovieSceneExecutionToken
{
    FZWMovieSceneDialogueExecutionToken(TObjectPtr<const UZWMovieSceneDialogueSection> InDialogueSection, const FZWDialogueData& EvaluatedDialogueData)
        : DialogueSection(MoveTemp(InDialogueSection))
        , DialogueData(EvaluatedDialogueData)
    {
    }

    virtual void Execute(const FMovieSceneContext& Context, const FMovieSceneEvaluationOperand& Operand, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) override
    {
        FZWMovieSceneDialoguePersistentData& DialoguePersistentData = PersistentData.GetOrAddSectionData<FZWMovieSceneDialoguePersistentData>();

        const UObject* PlaybackContext = Player.GetPlaybackContext();
        UWorld* World = PlaybackContext ? PlaybackContext->GetWorld() : nullptr;
        UZWMovieSceneDialogueSubsystem* DialogueSubsystem = World ? UGameInstance:: GetSubsystem<UZWMovieSceneDialogueSubsystem>(World->GetGameInstance()) : nullptr;

        if (!DialoguePersistentData.Token)
        {
            if (DialogueSubsystem != nullptr)
            {
                DialoguePersistentData.Token = DialogueSubsystem->TriggerDialogueLine(DialogueData);
            }
        }

        if (DialoguePersistentData.Token->EventID.IsValid() && DialogueSubsystem != nullptr)
        {
            DialogueSubsystem->TickDialogue(DialoguePersistentData.Token->EventID, DialogueData.AudioData.PrecalculatedDuration);
        }
    }
    
private:
    TObjectPtr<const UZWMovieSceneDialogueSection> DialogueSection = nullptr;
    FZWDialogueData DialogueData;
};

FZWMovieSceneDialogueSectionTemplate::FZWMovieSceneDialogueSectionTemplate(const UZWMovieSceneDialogueSection& InSection) : Section(&InSection)
{
}

void FZWMovieSceneDialogueSectionTemplate::Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
    FZWMovieSceneDialogueEditorTemplatePersistentData& TemplatePersistentData = PersistentData.GetOrAddTrackData<FZWMovieSceneDialogueEditorTemplatePersistentData>();

    TemplatePersistentData.SequenceID = Operand.SequenceID;
}

void FZWMovieSceneDialogueSectionTemplate::Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const
{
    check(Section);

   // FZWDialogueDetails DialogueData;
    //TODO: Check if refering to node is necessary. Might be in the future.
    //DialogueData = Section->EventID;
    //DialogueData.SpeakerID = Section->Speaker;
    //DialogueData.Speaker = Section->LocalizedSpeaker;
    //DialogueData.DialogueText = Section->DialogueText;

    //Commented out with the TODO above
    /*if (!DialogueData.SectionID.IsValid())
    {
        if (const UFlowNode* Node = Section->GetTypedOuter<UFlowNode>())
        {
            DialogueData.SectionID = Node->GetGuid();
        }
    }*/

    ExecutionTokens.Add(FZWMovieSceneDialogueExecutionToken(Section, Section->DialogueData));
}

void FZWMovieSceneDialogueSectionTemplate::SetupOverrides()
{
    EnableOverrides(RequiresTearDownFlag);
}

void FZWMovieSceneDialogueSectionTemplate::TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const
{
}

#undef LOCTEXT_NAMESPACE
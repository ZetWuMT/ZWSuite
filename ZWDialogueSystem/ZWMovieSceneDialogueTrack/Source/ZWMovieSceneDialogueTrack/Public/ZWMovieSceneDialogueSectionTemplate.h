#pragma once

#include "CoreMinimal.h"
#include "Evaluation/MovieSceneEvalTemplate.h"
#include "ZWMovieSceneDialogueSectionTemplate.generated.h"

class UZWMovieSceneDialogueSection;

USTRUCT()
struct FZWMovieSceneDialogueSectionTemplate : public FMovieSceneEvalTemplate
{
    GENERATED_BODY()

    FZWMovieSceneDialogueSectionTemplate() {}
    FZWMovieSceneDialogueSectionTemplate(const UZWMovieSceneDialogueSection& Section);

    virtual UScriptStruct& GetScriptStructImpl() const override { return *StaticStruct(); }

    virtual void Initialize(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;
    virtual void Evaluate(const FMovieSceneEvaluationOperand& Operand, const FMovieSceneContext& Context, const FPersistentEvaluationData& PersistentData, FMovieSceneExecutionTokens& ExecutionTokens) const override;
    virtual void SetupOverrides() override;
    virtual void TearDown(FPersistentEvaluationData& PersistentData, IMovieScenePlayer& Player) const override;

    UPROPERTY()
    TObjectPtr<const UZWMovieSceneDialogueSection> Section = nullptr;
};
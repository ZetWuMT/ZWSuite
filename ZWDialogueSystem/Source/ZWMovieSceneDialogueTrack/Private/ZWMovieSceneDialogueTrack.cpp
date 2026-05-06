// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWMovieSceneDialogueTrack.h"
#include "ZWMovieSceneDialogueSection.h"
#include "ZWMovieSceneDialogueSectionTemplate.h"
#include "Evaluation/MovieSceneEvaluationTrack.h"
#include "Compilation/IMovieSceneTemplateGenerator.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "IMovieSceneTracksModule.h"

#define LOCTEXT_NAMESPACE "ZWMovieSceneDialogueTrack"

void UZWMovieSceneDialogueTrack::AddSection(UMovieSceneSection& Section)
{
	Sections.Add(&Section);
}

bool UZWMovieSceneDialogueTrack::SupportsType(TSubclassOf<UMovieSceneSection> SectionClass) const
{
	return SectionClass->IsChildOf(UZWMovieSceneDialogueSection::StaticClass());
}

const TArray<UMovieSceneSection*>& UZWMovieSceneDialogueTrack::GetAllSections() const
{
	return Sections;
}

bool UZWMovieSceneDialogueTrack::HasSection(const UMovieSceneSection& Section) const
{
	return Sections.Contains(&Section);
}

bool UZWMovieSceneDialogueTrack::IsEmpty() const
{
	return (Sections.Num() == 0);
}

void UZWMovieSceneDialogueTrack::RemoveAllAnimationData()
{
	Sections.Empty();
}

void UZWMovieSceneDialogueTrack::RemoveSection(UMovieSceneSection& Section)
{
	Sections.Remove(&Section);
}

void UZWMovieSceneDialogueTrack::RemoveSectionAt(int32 SectionIndex)
{
	Sections.RemoveAt(SectionIndex);
}

UMovieSceneSection* UZWMovieSceneDialogueTrack::CreateNewSection()
{
	return NewObject<UZWMovieSceneDialogueSection>(this, NAME_None, RF_Transactional);
}



#if WITH_EDITORONLY_DATA
FText UZWMovieSceneDialogueTrack::GetDisplayName() const
{
	return LOCTEXT("TrackName", "Dialogue");
}
#endif

FMovieSceneEvalTemplatePtr UZWMovieSceneDialogueTrack::CreateTemplateForSection(const UMovieSceneSection& InSection) const
{
	if (InSection.IsA<UZWMovieSceneDialogueSection>())
	{
		return FZWMovieSceneDialogueSectionTemplate(*CastChecked<const UZWMovieSceneDialogueSection>(&InSection));
	}
	return FMovieSceneEvalTemplatePtr();
}

#undef LOCTEXT_NAMESPACE
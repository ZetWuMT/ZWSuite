// Copyright https://github.com/MothCocoon/DialogueGraph/graphs/contributors

#include "ZWMovieSceneDialogueTrackEditor.h"
#include "ZWMovieSceneDialogueSection.h"

#include "ZWMovieSceneDialogueTrack.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ISequencerSection.h"
#include "LevelSequence.h"
#include "MovieSceneSequenceEditor.h"
#include "Sections/MovieSceneEventSection.h"
#include "SequencerUtilities.h"

#define LOCTEXT_NAMESPACE "FZWMovieSceneDialogueTrackEditorModule"

TSharedRef<ISequencerTrackEditor> FZWDialogueTrackEditor::CreateTrackEditor(TSharedRef<ISequencer> InSequencer)
{
	return MakeShareable(new FZWDialogueTrackEditor(InSequencer));
}

TSharedRef<ISequencerSection> FZWDialogueTrackEditor::MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding)
{
	return MakeShareable(new FZWDialogueSection(SectionObject, GetSequencer()));
}

FZWDialogueTrackEditor::FZWDialogueTrackEditor(TSharedRef<ISequencer> InSequencer)
	: FMovieSceneTrackEditor(InSequencer)
{
}

void FZWDialogueTrackEditor::AddDialogueSubMenu(FMenuBuilder& MenuBuilder)
{

}

void FZWDialogueTrackEditor::BuildAddTrackMenu(FMenuBuilder& MenuBuilder)
{
	UMovieSceneSequence* RootMovieSceneSequence = GetSequencer()->GetRootMovieSceneSequence();
	const FMovieSceneSequenceEditor* SequenceEditor = FMovieSceneSequenceEditor::Find(RootMovieSceneSequence);

	if (SequenceEditor && SequenceEditor->SupportsEvents(RootMovieSceneSequence))
	{
		MenuBuilder.AddMenuEntry(
			LOCTEXT("AddTrack", "Dialogue Track"),
			LOCTEXT("AddTooltip", "Adds a new Dialogue track."),
			FSlateIcon(FAppStyle::GetAppStyleSetName(), "Sequencer.Tracks.Audio"),
			FUIAction(
				FExecuteAction::CreateRaw(this, &FZWDialogueTrackEditor::HandleAddDialogueTrackMenuEntryExecute)
			)
		);
	}
}

TSharedPtr<SWidget> FZWDialogueTrackEditor::BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params)
{
	check(Track);

	const TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	if (!SequencerPtr.IsValid())
	{
		return SNullWidget::NullWidget;
	}

	//TWeakObjectPtr<UMovieSceneTrack> WeakTrack = Track;
	const int32 RowIndex = Params.TrackInsertRowIndex;
	auto OnClickedCallback = [this, Track, RowIndex]() -> FReply
	{
		FZWDialogueTrackEditor::CreateNewSection(Track, RowIndex+1, UZWMovieSceneDialogueSection::StaticClass(), true);
		return FReply::Handled();
	};

	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.AutoWidth()
		.VAlign(VAlign_Center)
		[
			FSequencerUtilities::MakeAddButton(LOCTEXT("AddSection", "Section"), FOnClicked::CreateLambda(OnClickedCallback), Params.NodeIsHovered, GetSequencer())
		];
}

bool FZWDialogueTrackEditor::SupportsType(TSubclassOf<UMovieSceneTrack> Type) const
{
	return (Type == UZWMovieSceneDialogueTrack::StaticClass());
}

bool FZWDialogueTrackEditor::SupportsSequence(UMovieSceneSequence* InSequence) const
{
	return InSequence && InSequence->GetClass()->IsChildOf(ULevelSequence::StaticClass());
}

const FSlateBrush* FZWDialogueTrackEditor::GetIconBrush() const
{
	return FAppStyle::GetBrush("Sequencer.Tracks.Event");
}

void FZWDialogueTrackEditor::HandleAddDialogueTrackMenuEntryExecute()
{
	UMovieScene* FocusedMovieScene = GetFocusedMovieScene();

	if (FocusedMovieScene == nullptr)
	{
		return;
	}

	if (FocusedMovieScene->IsReadOnly())
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("AddDialogueTrack_Transaction", "Add Dialogue Track"));
	FocusedMovieScene->Modify();

	auto NewTrack = FocusedMovieScene->AddTrack<UZWMovieSceneDialogueTrack>();
	ensure(NewTrack);

	NewTrack->SetDisplayName(LOCTEXT("DialogueTrackName", "Dialogue"));

	if (GetSequencer().IsValid())
	{
		GetSequencer()->OnAddTrack(NewTrack, FGuid());
	}
}

//TODO: Dialogues can be on one track if they don't overlap
void FZWDialogueTrackEditor::CreateNewSection(UMovieSceneTrack* Track, const int32 RowIndex, UClass* SectionType, const bool bSelect) const
{
	const TSharedPtr<ISequencer> SequencerPtr = GetSequencer();
	FFrameNumber PlaybackEnd = UE::MovieScene::DiscreteExclusiveUpper(SequencerPtr->GetFocusedMovieSceneSequence()->GetMovieScene()->GetPlaybackRange());

	if (SequencerPtr.IsValid())
	{
		const UMovieScene* FocusedMovieScene = GetFocusedMovieScene();
		const FQualifiedFrameTime CurrentTime = SequencerPtr->GetLocalTime();

		FScopedTransaction Transaction(LOCTEXT("CreateNewDialogueSectionTransactionText", "Add Dialogue Section"));

		UMovieSceneSection* NewSection = NewObject<UMovieSceneSection>(Track, SectionType);
		check(NewSection);

		int32 OverlapPriority = 0;
		for (UMovieSceneSection* Section : Track->GetAllSections())
		{
			if (Section->GetRowIndex() >= RowIndex)
			{
				Section->SetRowIndex(Section->GetRowIndex() + 1);
			}
			OverlapPriority = FMath::Max(Section->GetOverlapPriority() + 1, OverlapPriority);
		}

		Track->Modify();

		NewSection->SetRange(TRange<FFrameNumber>(CurrentTime.Time.FrameNumber, PlaybackEnd));
		NewSection->SetOverlapPriority(OverlapPriority);
		NewSection->SetRowIndex(RowIndex);

		Track->AddSection(*NewSection);
		Track->UpdateEasing();

		if (bSelect)
		{
			SequencerPtr->EmptySelection();
			SequencerPtr->SelectSection(NewSection);
			SequencerPtr->ThrobSectionSelection();
		}

		SequencerPtr->NotifyMovieSceneDataChanged(EMovieSceneDataChangeType::MovieSceneStructureItemAdded);
	}
}
//@TODO: ColorTint do zmiany
int32 FZWDialogueSection::OnPaintSection(FSequencerSectionPainter& InPainter) const
{	
	return InPainter.PaintSectionBackground(FColor(134, 103, 106, 150));
}

UMovieSceneSection* FZWDialogueSection::GetSectionObject() 
{
	return &Section;
}

FText FZWDialogueSection::GetSectionTitle() const
{
	UZWMovieSceneDialogueSection* DialogueSection = Cast<UZWMovieSceneDialogueSection>(&Section);

	if (DialogueSection)
	{
		FString SectionText;

		if (!DialogueSection->Speaker.IsNone())
		{
			SectionText.Append(DialogueSection->Speaker.ToString());
			SectionText.Append(": ");
		}	
		SectionText.Append(DialogueSection->DialogueText.ToString());

	return FText::FromString(SectionText);
	}

	return FText();
}

float FZWDialogueSection::GetSectionHeight() const
{
	return 40.0f;
}
#undef LOCTEXT_NAMESPACE

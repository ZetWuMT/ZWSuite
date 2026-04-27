// Copyright https://github.com/MothCocoon/DialogueGraph/graphs/contributors

#pragma once

#include "Editor/Sequencer/Public/ISequencer.h"
#include "MovieSceneTrack.h"
#include "ISequencerSection.h"
#include "ISequencerTrackEditor.h"
#include "MovieSceneTrackEditor.h"
#include "ZWMovieSceneDialogueSection.h"
#include "SequencerSectionPainter.h"

class FMenuBuilder;

/**
* A property track editor for named events.
*/
class ZWMOVIESCENEDIALOGUETRACKEDITOR_API FZWDialogueTrackEditor : public FMovieSceneTrackEditor
{
public:
	/**
	 * Factory function to create an instance of this class (called by a sequencer).
	 *
	 * @param InSequencer The sequencer instance to be used by this tool.
	 * @return The new instance of this class.
	 */
	static TSharedRef<ISequencerTrackEditor> CreateTrackEditor(TSharedRef<ISequencer> InSequencer);

	/**
	 * Creates and initializes a new instance.
	 *
	 * @param InSequencer The sequencer instance to be used by this tool.
	 */
	FZWDialogueTrackEditor(TSharedRef<ISequencer> InSequencer);

	// ISequencerTrackEditor interface

	virtual void BuildAddTrackMenu(FMenuBuilder& MenuBuilder) override;
	virtual bool SupportsType(TSubclassOf<UMovieSceneTrack> Type) const override;
	virtual bool SupportsSequence(UMovieSceneSequence* InSequence) const override;
	virtual const FSlateBrush* GetIconBrush() const override;
	virtual TSharedPtr<SWidget> BuildOutlinerEditWidget(const FGuid& ObjectBinding, UMovieSceneTrack* Track, const FBuildEditWidgetParams& Params) override;

	//~ FPropertyTrackEditor interface
	virtual TSharedRef<ISequencerSection> MakeSectionInterface(UMovieSceneSection& SectionObject, UMovieSceneTrack& Track, FGuid ObjectBinding) override;

private:
	void AddDialogueSubMenu(FMenuBuilder& MenuBuilder);

	/** Callback for executing the "Add Dialogue Track" menu entry. */
	void HandleAddDialogueTrackMenuEntryExecute();

	void CreateNewSection(UMovieSceneTrack* Track, int32 RowIndex, UClass* SectionType, bool bSelect) const;
};

class ZWMOVIESCENEDIALOGUETRACKEDITOR_API FZWDialogueSection : public ISequencerSection
{
public:

	FZWDialogueSection(UMovieSceneSection& InSection, TWeakPtr<ISequencer> InSequencer)
		: Section( InSection )
	{
	}
	
	virtual int32 OnPaintSection(FSequencerSectionPainter& InPainter) const override;
	virtual UMovieSceneSection* GetSectionObject() override;
	virtual FText GetSectionTitle() const override;
	virtual float GetSectionHeight() const override;
	
	class UMovieSceneSection& Section;
};

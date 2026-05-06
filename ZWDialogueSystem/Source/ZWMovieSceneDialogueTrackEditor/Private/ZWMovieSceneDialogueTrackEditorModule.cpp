// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWMovieSceneDialogueTrackEditorModule.h"
#include "ZWMovieSceneDialogueTrackEditor.h"

#include "ISequencerModule.h"

#define LOCTEXT_NAMESPACE "FZWMovieSceneDialogueTrackEditorModule"

void FZWMovieSceneDialogueTrackEditorModule::StartupModule()
{
	// register Flow sequence track
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	DialogueTrackCreateEditorHandle = SequencerModule.RegisterTrackEditor(FOnCreateTrackEditor::CreateStatic(&FZWDialogueTrackEditor::CreateTrackEditor));
}

void FZWMovieSceneDialogueTrackEditorModule::ShutdownModule()
{
	// unregister track editors
	ISequencerModule& SequencerModule = FModuleManager::Get().LoadModuleChecked<ISequencerModule>("Sequencer");
	SequencerModule.UnRegisterTrackEditor(DialogueTrackCreateEditorHandle);
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FZWMovieSceneDialogueTrackEditorModule, ZWMovieSceneDialogueTrackEditor)
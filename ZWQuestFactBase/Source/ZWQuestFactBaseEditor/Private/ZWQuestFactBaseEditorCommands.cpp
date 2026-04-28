// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWQuestFactBaseEditorCommands.h"

#define LOCTEXT_NAMESPACE "FQuestFactBaseModule"

void FZWQuestFactBaseEditorCommands::RegisterCommands()
{
	UI_COMMAND(OpenPluginWindow, "QuestFactBase", "Bring up QuestFactBase window", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CreateNewFolder, "QuestFactBase", "Create new Folder", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(CreateNewFact, "QuestFactBase", "Create new Fact", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RemoveFact, "QuestFactBase", "Remove the selected Fact", EUserInterfaceActionType::Button, FInputChord());
	UI_COMMAND(RenameFact, "QuestFactBase", "Rename the selected Fact", EUserInterfaceActionType::Button, FInputChord(EKeys::F2));
	UI_COMMAND(ClearSelection, "QuestFactBase", "Clear the current selection", EUserInterfaceActionType::Button, FInputChord(EKeys::Escape));
}

#undef LOCTEXT_NAMESPACE

// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "ZWQuestFactBaseEditorStyle.h"

class FZWQuestFactBaseEditorCommands : public TCommands<FZWQuestFactBaseEditorCommands>
{
public:

	FZWQuestFactBaseEditorCommands()
		: TCommands<FZWQuestFactBaseEditorCommands>(TEXT("QuestFactBase"), NSLOCTEXT("Contexts", "QuestFactBase", "QuestFactBase Plugin"), NAME_None, FZWQuestFactBaseEditorStyle::GetStyleSetName())
	{
	}

	// TCommands<> interface
	virtual void RegisterCommands() override;

public:
	TSharedPtr<FUICommandInfo> OpenPluginWindow;
	TSharedPtr<FUICommandInfo> CreateNewFolder;
	TSharedPtr<FUICommandInfo> CreateNewFact;
	TSharedPtr<FUICommandInfo> RemoveFact;
	TSharedPtr<FUICommandInfo> RenameFact;
	TSharedPtr<FUICommandInfo> ClearSelection;
};
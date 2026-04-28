// Copyright Epic Games, Inc. All Rights Reserved.

#include "ZWQuestFactBaseEditorStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FZWQuestFactBaseEditorStyle::StyleInstance = nullptr;

void FZWQuestFactBaseEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FZWQuestFactBaseEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FZWQuestFactBaseEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("QuestFactBaseEditorStyle"));
	return StyleSetName;
}

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);

TSharedRef< FSlateStyleSet > FZWQuestFactBaseEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("QuestFactBaseEditorStyle"));
	//Style->SetContentRoot(IPluginManager::Get().FindPlugin("QuestFactBase")->GetBaseDir() / TEXT("Resources"));
	// engine assets
	Style->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate/"));

	const FVector2D Icon16(16.0f, 16.0f);
	const FVector2D Icon20(20.0f, 20.0f);
	const FVector2D Icon30(30.0f, 30.0f);
	const FVector2D Icon40(40.0f, 40.0f);
	const FVector2D Icon64(64.0f, 64.0f);

	Style->Set("QuestFactBase.OpenPluginWindow", new IMAGE_BRUSH_SVG(TEXT("Starship/Common/BrowseContent"), Icon20));
	Style->Set("QuestFactBase.Fact", new IMAGE_BRUSH_SVG(TEXT("Starship/AssetIcons/Object_16"), Icon16));
	Style->Set("QuestFactBase.FolderClosed", new IMAGE_BRUSH(TEXT("Icons/FolderClosed"), Icon16));
	Style->Set("QuestFactBase.FolderOpen", new IMAGE_BRUSH(TEXT("Icons/FolderOpen"), Icon16));

	return Style;
}

void FZWQuestFactBaseEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FZWQuestFactBaseEditorStyle::Get()
{
	return *StyleInstance;
}

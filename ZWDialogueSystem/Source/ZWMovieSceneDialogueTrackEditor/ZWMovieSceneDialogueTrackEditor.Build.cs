// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZWMovieSceneDialogueTrackEditor : ModuleRules
{
	public ZWMovieSceneDialogueTrackEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "EditorSubsystem",
            "ZWMovieSceneDialogueTrack",
            "MessageLog"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "ApplicationCore",
            "AssetSearch",
            "AssetTools",
            "BlueprintGraph",
            "ClassViewer",
            "ContentBrowser",
            "Core",
            "CoreUObject",
            "DetailCustomizations",
            "DeveloperSettings",
            "EditorFramework",
            "EditorScriptingUtilities",
            "EditorStyle",
            "Engine",
            "GraphEditor",
            "InputCore",
            "Json",
            "JsonUtilities",
            "Kismet",
            "KismetWidgets",
            "LevelEditor",
            "LevelSequence",
            "MovieScene",
            "MovieSceneTools",
            "MovieSceneTracks",
            "Projects",
            "PropertyEditor",
            "PropertyPath",
            "RenderCore",
            "Sequencer",
            "Slate",
            "SlateCore",
            "SourceControl",
            "ToolMenus",
            "UnrealEd"
        });
    }
}

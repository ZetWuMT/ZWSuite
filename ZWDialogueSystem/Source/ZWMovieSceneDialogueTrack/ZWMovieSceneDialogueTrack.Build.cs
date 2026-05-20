// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZWMovieSceneDialogueTrack : ModuleRules
{
	public ZWMovieSceneDialogueTrack(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "LevelSequence",
            "UMG",
            "CommonUI",
            "ZWDialogueSystem"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "Core",
            "CoreUObject",
            "DeveloperSettings",
            "Engine",
            "GameplayTags",
            "MovieScene",
            "MovieSceneTracks",
            "Slate",
            "SlateCore", 
        });

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new[]
            {
                "MessageLog",
                "UnrealEd"
            });
        }
    }
}

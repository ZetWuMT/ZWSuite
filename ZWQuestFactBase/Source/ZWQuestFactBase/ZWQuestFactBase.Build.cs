// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ZWQuestFactBase : ModuleRules
{
	public ZWQuestFactBase(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
		
		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
		});			
		
		PrivateDependencyModuleNames.AddRange(
		new string[]
			{	
			"Projects",
			"InputCore",
			"ToolMenus",
			"CoreUObject",
			"Engine",
			"Slate",
			"SlateCore",
			}
		);

        if (Target.Type == TargetType.Editor)
        {
            PublicDependencyModuleNames.AddRange(new[]
            {
				"EditorFramework",
                "UnrealEd",
				"EditorScriptingUtilities",
				"PropertyEditor"
            });
        }
    }
}

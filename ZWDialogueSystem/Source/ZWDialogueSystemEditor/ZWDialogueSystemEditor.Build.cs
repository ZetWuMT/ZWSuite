using UnrealBuildTool;

public class ZWDialogueSystemEditor : ModuleRules
{
    public ZWDialogueSystemEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "ZWDialogueSystem",
                "BlueprintEditorLibrary"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "HTTP",
                "Json"
            }
        );
    }
}
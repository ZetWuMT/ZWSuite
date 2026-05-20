using UnrealBuildTool;

public class ZWDialogueSystem : ModuleRules
{
    public ZWDialogueSystem(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core", 
                "CommonUI",
                "GameplayTags",
                "DeveloperSettings", 
                "RuntimeAudioImporter"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore",
                "UMG",
            }
        );
    }
}
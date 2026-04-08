using UnrealBuildTool;

public class ZWGameplayActionsEditor : ModuleRules
{
    public ZWGameplayActionsEditor(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "UnrealEd",  
                "CoreUObject",      
                "Engine",
                "BlueprintGraph",   
                "KismetCompiler",
                "Core",
                "AssetDefinition"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "ZWGameplayActions",
                "Slate",
                "SlateCore",
                "AssetTools"
            }
        );
    }
}
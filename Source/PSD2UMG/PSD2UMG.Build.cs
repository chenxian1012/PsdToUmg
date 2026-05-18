using UnrealBuildTool;

public class PSD2UMG : ModuleRules
{
    public PSD2UMG(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bUseUnity = true;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "UMG"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "UnrealEd", "UMGEditor", "Slate", "SlateCore", "RenderCore", "ImageWrapper",
            "AssetTools", "AssetRegistry", "AssetDefinition", "ToolMenus",
            "DeveloperSettings", "MessageLog", "Json", "JsonUtilities",
            "KismetCompiler", "BlueprintGraph", "Projects", "psd_sdk"
        });

        if (Target.Type == TargetType.Editor)
        {
            PrivateDependencyModuleNames.Add("EditorSubsystem");
        }

        if (Target.bBuildEditor && Target.Platform == UnrealTargetPlatform.Win64)
        {
            PrivateDependencyModuleNames.Add("CommonUI");
        }

        PrivateIncludePaths.AddRange(new[]
        {
            "PSD2UMG/Private",
            "PSD2UMG/Private/Importer",
            "PSD2UMG/Private/Schema",
            "PSD2UMG/Private/Builder",
            "PSD2UMG/Private/Settings",
            "PSD2UMG/Private/Asset"
        });
    }
}

using UnrealBuildTool;
using System.IO;

public class PSD2UMGTests : ModuleRules
{
    public PSD2UMGTests(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        CppStandard = CppStandardVersion.Cpp20;

        PublicDependencyModuleNames.AddRange(new[]
        {
            "Core", "CoreUObject", "Engine", "UMG", "PSD2UMG"
        });

        PrivateDependencyModuleNames.AddRange(new[]
        {
            "UnrealEd", "UMGEditor", "Json", "JsonUtilities",
            "AssetTools", "AssetRegistry", "BlueprintGraph", "KismetCompiler",
            "psd_sdk", "Projects"
        });

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "PSD2UMG", "Private"));
    }
}

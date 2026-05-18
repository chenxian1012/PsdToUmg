using UnrealBuildTool;
using System.IO;

public class psd_sdk : ModuleRules
{
    public psd_sdk(ReadOnlyTargetRules Target) : base(Target)
    {
        Type = ModuleType.CPlusPlus;
        PCHUsage = PCHUsageMode.NoPCHs;
        bUseUnity = false;
        bEnableExceptions = false;
        bEnableUndefinedIdentifierWarnings = false;
        ShadowVariableWarningLevel = WarningLevel.Off;
        bDisableStaticAnalysis = true;

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Includes"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Includes", "Psd"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Source"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Source", "Psd"));
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));

        PublicDependencyModuleNames.Add("Core");
    }
}

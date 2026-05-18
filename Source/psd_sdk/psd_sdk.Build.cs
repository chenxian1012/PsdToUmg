using UnrealBuildTool;
using System.IO;

public class psd_sdk : ModuleRules
{
    public psd_sdk(ReadOnlyTargetRules Target) : base(Target)
    {
        // Header-only module: the .cpp/.c implementation files have been moved
        // into the PSD2UMG module under Private/ThirdParty/psd_sdk/ so that the
        // upstream-undecorated functions are compiled directly into the consumer
        // (psd_sdk has no __declspec(dllexport) decoration, so the symbols would
        // otherwise be invisible across DLL boundaries on Windows).
        //
        // PsdSdkModule.cpp under Private/ remains so that IMPLEMENT_MODULE keeps
        // this module loadable by name (per the .uplugin Modules entry).
        Type = ModuleType.CPlusPlus;
        PCHUsage = PCHUsageMode.NoPCHs;
        bUseUnity = false;
        bEnableExceptions = false;
        bEnableUndefinedIdentifierWarnings = false;
        ShadowVariableWarningLevel = WarningLevel.Off;
        bDisableStaticAnalysis = true;

        PublicSystemIncludePaths.Add(Path.Combine(ModuleDirectory, "Includes"));

        PublicDependencyModuleNames.Add("Core");
    }
}

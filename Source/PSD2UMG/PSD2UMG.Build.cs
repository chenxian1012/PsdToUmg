using UnrealBuildTool;
using System.IO;

public class PSD2UMG : ModuleRules
{
    public PSD2UMG(ReadOnlyTargetRules Target) : base(Target)
    {
        // Note: unity is disabled module-wide because Private/ThirdParty/psd_sdk
        // (vendored MolecularMatters/psd_sdk source files) is allergic to being
        // merged with UE's macro-heavy headers and uses its own internal
        // PsdPch.h include guard pattern.
        PCHUsage = PCHUsageMode.NoPCHs;
        CppStandard = CppStandardVersion.Cpp20;
        bUseUnity = false;
        bEnableUndefinedIdentifierWarnings = false;
        ShadowVariableWarningLevel = WarningLevel.Off;

        // psd_sdk still uses std::is_pod (deprecated in C++20). Silence the
        // STL deprecation warning so the build doesn't fail under -Werror.
        PrivateDefinitions.Add("_SILENCE_CXX20_IS_POD_DEPRECATION_WARNING=1");
        PrivateDefinitions.Add("_SILENCE_ALL_CXX20_DEPRECATION_WARNINGS=1");

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
            "PSD2UMG/Private/Builder",
            "PSD2UMG/Private/Settings",
            "PSD2UMG/Private/Asset"
        });

        // The vendored psd_sdk .cpp files compiled under Private/ThirdParty/psd_sdk
        // include their internal headers without the "Psd/" prefix (e.g.
        // #include "PsdPch.h"), so make the headers directory a private include
        // root just like upstream's CMake build does.
        //
        // IMPORTANT: do NOT add "PSD2UMG/Private/Schema" to the include path —
        // both our intermediate-representation header and psd_sdk's parser header
        // are named PsdDocument.h, and the schema dir on the include path would
        // shadow psd_sdk's. Consumers of our schema must include it as
        // "Schema/PsdDocument.h".
        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "..", "psd_sdk", "Includes", "Psd"));
    }
}

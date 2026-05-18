#include "Builder/StyleAssetBuilder.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "UObject/Package.h"
#include "UObject/UObjectGlobals.h"

namespace PSD2UMG
{
    namespace
    {
        UClass* FindClassByName(const TCHAR* Name)
        {
            // UE 5.1+ replacement for FindObject<UClass>(ANY_PACKAGE, Name).
            return FindFirstObject<UClass>(Name, EFindFirstObjectOptions::None);
        }

        FSoftObjectPath CreateOrGetStubAsset(const FString& PackagePath,
                                              const FString& AssetName,
                                              UClass* StyleClass)
        {
            // Treat "class not found" and "class is abstract (cannot be directly
            // instantiated as a stub)" the same way: return an empty soft path
            // and let callers fall back to "no style applied". UCommonTextStyle
            // and UCommonButtonStyle are abstract in CommonUI 5.7 — concrete
            // game-specific subclasses are expected. v2 will accept an override
            // concrete class via plugin settings.
            if (!StyleClass || StyleClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists))
            {
                return FSoftObjectPath();
            }

            UPackage* Package = CreatePackage(*PackagePath);
            UObject* Existing = StaticFindObject(StyleClass, Package, *AssetName);
            UObject* Asset = Existing
                ? Existing
                : NewObject<UObject>(Package, StyleClass, *AssetName, RF_Public | RF_Standalone);

            Asset->MarkPackageDirty();
            if (!Existing) FAssetRegistryModule::AssetCreated(Asset);
            return FSoftObjectPath(Asset);
        }
    }

    FSoftObjectPath FStyleAssetBuilder::GetOrCreateTextStyle(const FString& PackagePath,
                                                              const FString& AssetName,
                                                              const FTextStyleSpec& /*Spec*/)
    {
        UClass* StyleClass = FindClassByName(TEXT("CommonTextStyle"));
        return CreateOrGetStubAsset(PackagePath, AssetName, StyleClass);
    }

    FSoftObjectPath FStyleAssetBuilder::GetOrCreateButtonStyle(const FString& PackagePath,
                                                                const FString& AssetName,
                                                                const FSlateBrushSpec& /*Normal*/,
                                                                const FSlateBrushSpec& /*Hovered*/,
                                                                const FSlateBrushSpec& /*Pressed*/)
    {
        UClass* StyleClass = FindClassByName(TEXT("CommonButtonStyle"));
        return CreateOrGetStubAsset(PackagePath, AssetName, StyleClass);
    }
}

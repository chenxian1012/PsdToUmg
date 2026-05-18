#pragma once
#include "CoreMinimal.h"
#include "Schema/WidgetSpec.h"
#include "UObject/SoftObjectPath.h"

namespace PSD2UMG
{
    class FStyleAssetBuilder
    {
    public:
        // Returns a soft path to a UCommonTextStyle asset (or empty if CommonUI is not loaded).
        // Asset properties default-initialize in v1; populate manually in the editor.
        static PSD2UMG_API FSoftObjectPath GetOrCreateTextStyle(const FString& PackagePath,
                                                                 const FString& AssetName,
                                                                 const FTextStyleSpec& Spec);

        // Returns a soft path to a UCommonButtonStyle asset (or empty if CommonUI is not loaded).
        static PSD2UMG_API FSoftObjectPath GetOrCreateButtonStyle(const FString& PackagePath,
                                                                   const FString& AssetName,
                                                                   const FSlateBrushSpec& Normal,
                                                                   const FSlateBrushSpec& Hovered,
                                                                   const FSlateBrushSpec& Pressed);
    };
}

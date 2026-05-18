#pragma once

#include "CoreMinimal.h"
#include "Schema/WidgetSpec.h"

namespace PSD2UMG
{
    struct FParsedLayerName
    {
        FString      BaseName;
        EWidgetType  Type = EWidgetType::Image;
        bool         bUseCommonUI = true;
        bool         bNineSlice = false;
        FPsdMargin   NineSliceMargin;
        EButtonState ButtonState = EButtonState::Normal;
        EProgressPart ProgressPart = EProgressPart::Background;
        EAnchorPreset Anchor = EAnchorPreset::Auto;
        FVector2D    SizeBoxDims = FVector2D::ZeroVector;
        FString      LinkedPsdRelPath;   // populated when #linkedpsd(...) is present
        TArray<FString> Warnings;
    };

    class FPsdNamingParser
    {
    public:
        /**
         * Parse a layer name like `Name#tag1(args)#tag2(args)`.
         * Returns false only on malformed input; unknown tags become Warnings.
         */
        static PSD2UMG_API bool Parse(const FString& LayerName, FParsedLayerName& Out);
    };
}

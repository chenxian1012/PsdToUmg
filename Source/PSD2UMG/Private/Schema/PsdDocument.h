#pragma once

#include "CoreMinimal.h"

namespace PSD2UMG
{
    enum class ELayerKind : uint8
    {
        Group,
        Raster,
        Text,
        LinkedPsd
    };

    enum class EBlendMode : uint8
    {
        Normal,
        Multiply,
        Screen,
        Overlay,
        Add,
        Subtract,
        Unknown
    };

    enum class EPsdTextJustify : uint8
    {
        Left,
        Center,
        Right
    };

    struct FPsdTextRun
    {
        FString  Text;
        FString  FontFamily;
        float    FontSizePx = 12.0f;
        FLinearColor Color = FLinearColor::Black;
        EPsdTextJustify Justify = EPsdTextJustify::Left;
        bool     bBold = false;
        bool     bItalic = false;
    };

    struct FPsdLinkedRef
    {
        FString  RelPath;       // Relative path from the parent PSD, e.g. "Avatar.psd"
        FString  AbsPath;       // Resolved absolute path; empty if unresolved
    };

    struct FPsdLayer
    {
        FString       Name;
        FBox2D        Bounds = FBox2D(ForceInit);
        float         Opacity = 1.0f;
        EBlendMode    Blend = EBlendMode::Normal;
        ELayerKind    Kind = ELayerKind::Raster;
        bool          bVisible = true;

        // Raster: RGBA8 row-major, length = (Bounds.Width * Bounds.Height * 4).
        // Empty for Group / Text / LinkedPsd.
        TArray<uint8> Pixels;

        // Text only.
        FPsdTextRun   TextRun;

        // LinkedPsd only.
        FPsdLinkedRef LinkedRef;

        // Group only (populated by PsdSchemaResolver from psd_sdk's flat list).
        TArray<FPsdLayer> Children;
    };

    struct FPsdDocument
    {
        FIntPoint  CanvasSize = FIntPoint::ZeroValue;
        int32      ColorDepth = 8;
        TArray<FPsdLayer> Layers;
    };
}

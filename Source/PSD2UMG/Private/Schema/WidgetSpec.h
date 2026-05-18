#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

namespace PSD2UMG
{
    enum class EWidgetType : uint8
    {
        Canvas,
        Image,
        Button,
        ProgressBar,
        Text,
        SizeBox,
        ScaleBox,
        NamedSlot,
        SubWidget,
        Skip
    };

    enum class EButtonState : uint8 { Normal, Hovered, Pressed, Disabled };
    enum class EProgressPart : uint8 { Background, Fill, Marquee };
    enum class EAnchorPreset : uint8 { Auto, TL, T, TR, L, C, R, BL, B, BR, Stretch };

    // Schema-local mirror of FMargin (L, T, R, B). Builder translates at the seam.
    struct FPsdMargin
    {
        float Left = 0.f;
        float Top = 0.f;
        float Right = 0.f;
        float Bottom = 0.f;
    };

    struct FSlateBrushSpec
    {
        FName    TextureAssetName;        // T_<LayerName>
        bool     bNineSlice = false;
        FPsdMargin Margin;                // when bNineSlice
    };

    struct FTextStyleSpec
    {
        FString  Text;
        FString  FontFamily;
        float    FontSizePx = 12.0f;
        FLinearColor Color = FLinearColor::White;
        // Note: justify lives on FPsdLayer.TextRun.Justify and propagates via Resolver.
    };

    struct FWidgetSpec
    {
        FName WidgetName;
        EWidgetType Type = EWidgetType::Canvas;
        bool   bUseCommonUI = true;

        FBox2D Bounds = FBox2D(ForceInit);
        EAnchorPreset Anchor = EAnchorPreset::Auto;

        FSlateBrushSpec Brush;
        FTextStyleSpec  TextStyle;

        EButtonState ButtonState = EButtonState::Normal;
        EProgressPart ProgressPart = EProgressPart::Background;

        // Per-button-state brushes (only on the merged Button spec; empty otherwise).
        TMap<EButtonState, FSlateBrushSpec> ButtonStates;

        // For SubWidget (LinkedPsd): the child WBP asset name (e.g. "WBP_Avatar").
        FName  SubWidgetAssetName;

        // Optional override from .psd.json that takes precedence over generated style assets.
        FSoftObjectPath StyleAssetRef;

        TArray<FWidgetSpec> Children;
    };
}

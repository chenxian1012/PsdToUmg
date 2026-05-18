#include "Schema/PsdNamingParser.h"

namespace PSD2UMG
{
    namespace
    {
        bool ParseTagBody(const FString& Tag, const FString& Args, FParsedLayerName& Out)
        {
            if (Tag == TEXT("image"))          { Out.Type = EWidgetType::Image; return true; }
            if (Tag == TEXT("text"))           { Out.Type = EWidgetType::Text;  return true; }
            if (Tag == TEXT("button"))         { Out.Type = EWidgetType::Button; Out.ButtonState = EButtonState::Normal; return true; }
            if (Tag == TEXT("button_normal"))  { Out.Type = EWidgetType::Button; Out.ButtonState = EButtonState::Normal;   return true; }
            if (Tag == TEXT("button_hovered")) { Out.Type = EWidgetType::Button; Out.ButtonState = EButtonState::Hovered;  return true; }
            if (Tag == TEXT("button_pressed")) { Out.Type = EWidgetType::Button; Out.ButtonState = EButtonState::Pressed;  return true; }
            if (Tag == TEXT("button_disabled")){ Out.Type = EWidgetType::Button; Out.ButtonState = EButtonState::Disabled; return true; }
            if (Tag == TEXT("progress"))
            {
                Out.Type = EWidgetType::ProgressBar;
                if      (Args == TEXT("bg"))      Out.ProgressPart = EProgressPart::Background;
                else if (Args == TEXT("fill"))    Out.ProgressPart = EProgressPart::Fill;
                else if (Args == TEXT("marquee")) Out.ProgressPart = EProgressPart::Marquee;
                return true;
            }
            if (Tag == TEXT("9slice"))
            {
                Out.bNineSlice = true;
                TArray<FString> Nums;
                Args.ParseIntoArray(Nums, TEXT(","));
                if (Nums.Num() == 4)
                {
                    Out.NineSliceMargin.Left   = FCString::Atof(*Nums[0]);
                    Out.NineSliceMargin.Right  = FCString::Atof(*Nums[1]);
                    Out.NineSliceMargin.Top    = FCString::Atof(*Nums[2]);
                    Out.NineSliceMargin.Bottom = FCString::Atof(*Nums[3]);
                }
                return true;
            }
            if (Tag == TEXT("sizebox"))
            {
                Out.Type = EWidgetType::SizeBox;
                TArray<FString> Parts;
                Args.ParseIntoArray(Parts, TEXT(","));
                for (const FString& P : Parts)
                {
                    FString K, V;
                    if (P.Split(TEXT("="), &K, &V))
                    {
                        if (K == TEXT("W")) Out.SizeBoxDims.X = FCString::Atof(*V);
                        if (K == TEXT("H")) Out.SizeBoxDims.Y = FCString::Atof(*V);
                    }
                }
                return true;
            }
            if (Tag == TEXT("scalebox")) { Out.Type = EWidgetType::ScaleBox; return true; }
            if (Tag == TEXT("slot"))     { Out.Type = EWidgetType::NamedSlot; return true; }
            if (Tag == TEXT("anchor"))
            {
                if      (Args == TEXT("TL"))      Out.Anchor = EAnchorPreset::TL;
                else if (Args == TEXT("T"))       Out.Anchor = EAnchorPreset::T;
                else if (Args == TEXT("TR"))      Out.Anchor = EAnchorPreset::TR;
                else if (Args == TEXT("L"))       Out.Anchor = EAnchorPreset::L;
                else if (Args == TEXT("C"))       Out.Anchor = EAnchorPreset::C;
                else if (Args == TEXT("R"))       Out.Anchor = EAnchorPreset::R;
                else if (Args == TEXT("BL"))      Out.Anchor = EAnchorPreset::BL;
                else if (Args == TEXT("B"))       Out.Anchor = EAnchorPreset::B;
                else if (Args == TEXT("BR"))      Out.Anchor = EAnchorPreset::BR;
                else if (Args == TEXT("Stretch")) Out.Anchor = EAnchorPreset::Stretch;
                return true;
            }
            if (Tag == TEXT("linkedpsd")) { Out.Type = EWidgetType::SubWidget; Out.LinkedPsdRelPath = Args; return true; }
            if (Tag == TEXT("vanilla"))   { Out.bUseCommonUI = false; return true; }
            if (Tag == TEXT("skip"))      { Out.Type = EWidgetType::Skip; return true; }
            return false;
        }
    }

    bool FPsdNamingParser::Parse(const FString& LayerName, FParsedLayerName& Out)
    {
        const int32 FirstHash = LayerName.Find(TEXT("#"));
        Out.BaseName = (FirstHash == INDEX_NONE)
            ? LayerName
            : LayerName.Left(FirstHash);
        Out.BaseName.TrimStartAndEndInline();
        if (FirstHash == INDEX_NONE) return true;

        TArray<FString> Tags;
        LayerName.RightChop(FirstHash + 1).ParseIntoArray(Tags, TEXT("#"));
        for (const FString& Raw : Tags)
        {
            FString Tag = Raw;
            FString Args;
            const int32 P1 = Raw.Find(TEXT("("));
            if (P1 != INDEX_NONE && Raw.EndsWith(TEXT(")")))
            {
                Tag  = Raw.Left(P1);
                Args = Raw.Mid(P1 + 1, Raw.Len() - P1 - 2);
            }
            if (!ParseTagBody(Tag, Args, Out))
            {
                Out.Warnings.Add(FString::Printf(TEXT("unknown tag '#%s', treated as no-op"), *Tag));
            }
        }
        return true;
    }
}

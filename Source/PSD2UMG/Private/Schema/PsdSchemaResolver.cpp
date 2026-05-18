#include "Schema/PsdSchemaResolver.h"
#include "Schema/PsdNamingParser.h"

#include "Misc/Paths.h"

namespace PSD2UMG
{
    namespace
    {
        EAnchorPreset DefaultAnchorFromCenter(const FBox2D& Layer, const FBox2D& Parent)
        {
            const float Cx = (Layer.Min.X + Layer.Max.X) * 0.5f;
            const float Cy = (Layer.Min.Y + Layer.Max.Y) * 0.5f;
            const float Px = FMath::Max(Parent.GetSize().X, 1.0f);
            const float Py = FMath::Max(Parent.GetSize().Y, 1.0f);
            const float Tx = (Cx - Parent.Min.X) / Px;
            const float Ty = (Cy - Parent.Min.Y) / Py;
            auto Slot = [](float V) { return V < 0.25f ? 0 : (V > 0.75f ? 2 : 1); };
            const int SX = Slot(Tx), SY = Slot(Ty);
            static const EAnchorPreset Grid[3][3] = {
                { EAnchorPreset::TL, EAnchorPreset::T, EAnchorPreset::TR },
                { EAnchorPreset::L,  EAnchorPreset::C, EAnchorPreset::R  },
                { EAnchorPreset::BL, EAnchorPreset::B, EAnchorPreset::BR },
            };
            return Grid[SY][SX];
        }

        FName SubWidgetAssetNameFromRelPath(const FString& RelPath)
        {
            return FName(*(FString(TEXT("WBP_")) + FPaths::GetBaseFilename(RelPath)));
        }

        void ApplySidecarOverrides(const FResolveContext& Ctx, const FString& BaseName,
                                    FWidgetSpec& OutSpec)
        {
            const FPsdSidecarLayerOverride* Ov = Ctx.Sidecar.PerLayer.Find(BaseName);
            if (!Ov) return;
            if (OutSpec.Type == EWidgetType::Button && !Ov->CommonButtonStyle.IsNull())
            {
                OutSpec.StyleAssetRef = Ov->CommonButtonStyle;
            }
            else if (OutSpec.Type == EWidgetType::Text && !Ov->TextStyle.IsNull())
            {
                OutSpec.StyleAssetRef = Ov->TextStyle;
            }
        }

        FWidgetSpec MakeBaseSpec(const FPsdLayer& Layer, const FParsedLayerName& P,
                                  const FBox2D& CanvasBounds, const FResolveContext& Ctx)
        {
            FWidgetSpec S;
            S.WidgetName = FName(*P.BaseName);
            S.Bounds = Layer.Bounds;
            S.Anchor = (P.Anchor == EAnchorPreset::Auto)
                ? DefaultAnchorFromCenter(Layer.Bounds, CanvasBounds)
                : P.Anchor;
            S.bUseCommonUI = P.bUseCommonUI
                          && Ctx.Sidecar.Globals.bUseCommonUI
                          && Ctx.ProjectDefaultUseCommonUI;
            S.Brush.bNineSlice = P.bNineSlice;
            S.Brush.Margin = P.NineSliceMargin;
            S.Brush.TextureAssetName = FName(*(FString(TEXT("T_")) + Layer.Name));
            return S;
        }
    }

    void FPsdSchemaResolver::Resolve(const FPsdDocument& Doc, FResolveContext& Ctx,
                                      FWidgetSpec& OutRoot, TArray<FString>& OutWarnings)
    {
        OutRoot.Type = EWidgetType::Canvas;
        OutRoot.WidgetName = TEXT("Root");
        const FBox2D CanvasBounds(FVector2D::ZeroVector,
                                   FVector2D(Doc.CanvasSize.X, Doc.CanvasSize.Y));
        OutRoot.Bounds = CanvasBounds;

        // First pass: collect per-base-name button layers for state aggregation.
        // BaseName -> array of (layer index, button state).
        struct FButtonGroupEntry { int32 LayerIndex; EButtonState State; };
        TMap<FString, TArray<FButtonGroupEntry>> ButtonGroups;
        for (int32 i = 0; i < Doc.Layers.Num(); ++i)
        {
            FParsedLayerName P;
            FPsdNamingParser::Parse(Doc.Layers[i].Name, P);
            if (P.Type == EWidgetType::Button)
            {
                ButtonGroups.FindOrAdd(P.BaseName).Add({ i, P.ButtonState });
            }
        }

        // Track which layer indices we've consumed via button aggregation.
        TSet<int32> ConsumedAsButtonState;

        // Second pass: emit one spec per layer (or one spec per button group).
        for (int32 i = 0; i < Doc.Layers.Num(); ++i)
        {
            if (ConsumedAsButtonState.Contains(i)) continue;

            const FPsdLayer& Layer = Doc.Layers[i];
            FParsedLayerName P;
            FPsdNamingParser::Parse(Layer.Name, P);
            for (const FString& W : P.Warnings)
            {
                OutWarnings.Add(FString::Printf(TEXT("%s: %s"), *Layer.Name, *W));
            }

            if (P.Type == EWidgetType::Skip) continue;

            // Button aggregation: produce one merged spec for the whole group.
            if (P.Type == EWidgetType::Button)
            {
                const TArray<FButtonGroupEntry>* Group = ButtonGroups.Find(P.BaseName);
                if (!Group || Group->Num() == 0) continue;
                if ((*Group)[0].LayerIndex != i) continue; // already handled by an earlier index

                FWidgetSpec BtnSpec = MakeBaseSpec(Layer, P, CanvasBounds, Ctx);
                BtnSpec.Type = EWidgetType::Button;
                BtnSpec.WidgetName = FName(*P.BaseName);

                for (const FButtonGroupEntry& E : *Group)
                {
                    ConsumedAsButtonState.Add(E.LayerIndex);
                    const FPsdLayer& StateLayer = Doc.Layers[E.LayerIndex];
                    FSlateBrushSpec StateBrush;
                    StateBrush.TextureAssetName = FName(*(FString(TEXT("T_")) + StateLayer.Name));
                    BtnSpec.ButtonStates.Add(E.State, StateBrush);
                }
                ApplySidecarOverrides(Ctx, P.BaseName, BtnSpec);
                OutRoot.Children.Add(MoveTemp(BtnSpec));
                continue;
            }

            // LinkedPsd → SubWidget
            if (Layer.Kind == ELayerKind::LinkedPsd || P.Type == EWidgetType::SubWidget)
            {
                // RelPath comes from the parser if the layer name had #linkedpsd(...).
                // The reader will already have stripped the tag from Layer.Name, so prefer the parser output.
                FString RelPath = !P.LinkedPsdRelPath.IsEmpty()
                    ? P.LinkedPsdRelPath
                    : Layer.LinkedRef.RelPath;
                if (RelPath.IsEmpty())
                {
                    OutWarnings.Add(FString::Printf(TEXT("%s: linkedpsd without relpath, skipping"),
                                                     *Layer.Name));
                    continue;
                }
                // Cycle detection is keyed on absolute path; resolver doesn't know the
                // parent PSD's directory here, so it records the rel path. Factory layer
                // will re-key on AbsPath. We still emit the spec.
                FWidgetSpec Sub = MakeBaseSpec(Layer, P, CanvasBounds, Ctx);
                Sub.Type = EWidgetType::SubWidget;
                Sub.WidgetName = FName(*P.BaseName);
                Sub.SubWidgetAssetName = SubWidgetAssetNameFromRelPath(RelPath);
                OutRoot.Children.Add(MoveTemp(Sub));
                continue;
            }

            // Plain Image / Text / ProgressBar / SizeBox / ScaleBox / NamedSlot
            FWidgetSpec S = MakeBaseSpec(Layer, P, CanvasBounds, Ctx);
            S.Type = P.Type;
            if (P.Type == EWidgetType::Image && Layer.Kind == ELayerKind::Text)
            {
                // psd_sdk classifies most layers as Raster; text layers will be
                // promoted later via tag (#text). Keep simple for v1.
                S.Type = EWidgetType::Text;
                S.TextStyle.Text = Layer.TextRun.Text;
                S.TextStyle.FontFamily = Layer.TextRun.FontFamily;
                S.TextStyle.FontSizePx = Layer.TextRun.FontSizePx;
                S.TextStyle.Color = Layer.TextRun.Color;
            }
            else if (P.Type == EWidgetType::Text)
            {
                // Designer tagged #text but psd_sdk didn't see a text layer block.
                // Still emit a Text widget; TextRun.Text will be empty (sidecar/JSON can fill it).
                S.Type = EWidgetType::Text;
            }
            else if (P.Type == EWidgetType::ProgressBar)
            {
                S.ProgressPart = P.ProgressPart;
            }
            else if (P.Type == EWidgetType::SizeBox)
            {
                // SizeBoxDims override Bounds size if explicit args were given.
                if (P.SizeBoxDims.X > 0 || P.SizeBoxDims.Y > 0)
                {
                    const FVector2D Min = S.Bounds.Min;
                    const FVector2D Size = FVector2D(
                        P.SizeBoxDims.X > 0 ? P.SizeBoxDims.X : S.Bounds.GetSize().X,
                        P.SizeBoxDims.Y > 0 ? P.SizeBoxDims.Y : S.Bounds.GetSize().Y);
                    S.Bounds = FBox2D(Min, Min + Size);
                }
            }
            ApplySidecarOverrides(Ctx, P.BaseName, S);
            OutRoot.Children.Add(MoveTemp(S));
        }
    }
}

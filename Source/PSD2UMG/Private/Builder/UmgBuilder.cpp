#include "Builder/UmgBuilder.h"
#include "Builder/TextureBuilder.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/Image.h"
#include "Engine/Texture2D.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Layout/Margin.h"
#include "Styling/SlateBrush.h"
#include "UObject/Package.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintFactory.h"

namespace PSD2UMG
{
    namespace
    {
        FAnchors PresetToAnchors(EAnchorPreset P)
        {
            switch (P)
            {
                case EAnchorPreset::TL: return FAnchors(0,0,0,0);
                case EAnchorPreset::T:  return FAnchors(0.5f,0,0.5f,0);
                case EAnchorPreset::TR: return FAnchors(1,0,1,0);
                case EAnchorPreset::L:  return FAnchors(0,0.5f,0,0.5f);
                case EAnchorPreset::C:  return FAnchors(0.5f,0.5f,0.5f,0.5f);
                case EAnchorPreset::R:  return FAnchors(1,0.5f,1,0.5f);
                case EAnchorPreset::BL: return FAnchors(0,1,0,1);
                case EAnchorPreset::B:  return FAnchors(0.5f,1,0.5f,1);
                case EAnchorPreset::BR: return FAnchors(1,1,1,1);
                case EAnchorPreset::Stretch: return FAnchors(0,0,1,1);
                default: return FAnchors(0,0,0,0);
            }
        }

        FMargin ToFMargin(const FPsdMargin& M) { return FMargin(M.Left, M.Top, M.Right, M.Bottom); }

        UWidgetBlueprint* GetOrCreateWbp(const FString& PackagePath, const FString& Name)
        {
            const FString FullObjPath = PackagePath / (Name + TEXT(".") + Name);
            if (UWidgetBlueprint* Existing = LoadObject<UWidgetBlueprint>(nullptr, *FullObjPath))
            {
                return Existing;
            }

            UPackage* Package = CreatePackage(*(PackagePath / Name));
            UWidgetBlueprintFactory* Factory = NewObject<UWidgetBlueprintFactory>();
            Factory->ParentClass = UUserWidget::StaticClass();

            UObject* New = Factory->FactoryCreateNew(
                UWidgetBlueprint::StaticClass(),
                Package, *Name, RF_Public | RF_Standalone, nullptr, GWarn);
            UWidgetBlueprint* Wbp = Cast<UWidgetBlueprint>(New);
            if (Wbp)
            {
                FAssetRegistryModule::AssetCreated(Wbp);
                Wbp->MarkPackageDirty();
            }
            return Wbp;
        }

        void ApplyCanvasSlot(UWidget* Child, const FWidgetSpec& Spec)
        {
            if (!Child || !Child->Slot) return;
            UCanvasPanelSlot* Slot = Cast<UCanvasPanelSlot>(Child->Slot);
            if (!Slot) return;
            Slot->SetAnchors(PresetToAnchors(Spec.Anchor));
            Slot->SetPosition(Spec.Bounds.Min);
            Slot->SetSize(Spec.Bounds.GetSize());
        }

        UTexture2D* LoadTextureForSpec(UWidgetBlueprint* Wbp, const FName& TexAssetName)
        {
            if (TexAssetName.IsNone()) return nullptr;
            const FString WbpPackage = Wbp->GetOuter()->GetName();   // /Game/UI/PsdImport/<PsdName>/WBP_<Name>
            const FString PsdDir = FPaths::GetPath(WbpPackage);       // /Game/UI/PsdImport/<PsdName>
            const FString TexPath = PsdDir / (TexAssetName.ToString() + TEXT(".") + TexAssetName.ToString());
            return LoadObject<UTexture2D>(nullptr, *TexPath);
        }

        UWidget* BuildSpec(UWidgetBlueprint* Wbp, UPanelWidget* Parent, const FWidgetSpec& Spec);

        UWidget* BuildCanvas(UWidgetBlueprint* Wbp, UPanelWidget* Parent, const FWidgetSpec& Spec)
        {
            UCanvasPanel* Canvas = Wbp->WidgetTree->ConstructWidget<UCanvasPanel>(
                UCanvasPanel::StaticClass(), Spec.WidgetName);
            if (Parent) { Parent->AddChild(Canvas); ApplyCanvasSlot(Canvas, Spec); }
            else        { Wbp->WidgetTree->RootWidget = Canvas; }

            for (const FWidgetSpec& Child : Spec.Children)
            {
                BuildSpec(Wbp, Canvas, Child);
            }
            return Canvas;
        }

        UWidget* BuildImage(UWidgetBlueprint* Wbp, UPanelWidget* Parent, const FWidgetSpec& Spec)
        {
            UImage* Img = Wbp->WidgetTree->ConstructWidget<UImage>(UImage::StaticClass(), Spec.WidgetName);
            Parent->AddChild(Img);
            ApplyCanvasSlot(Img, Spec);

            if (UTexture2D* Tex = LoadTextureForSpec(Wbp, Spec.Brush.TextureAssetName))
            {
                FSlateBrush Brush;
                Brush.SetResourceObject(Tex);
                Brush.ImageSize = Spec.Bounds.GetSize();
                Brush.Margin    = ToFMargin(Spec.Brush.Margin);
                Brush.DrawAs    = Spec.Brush.bNineSlice ? ESlateBrushDrawType::Box
                                                          : ESlateBrushDrawType::Image;
                Img->SetBrush(Brush);
            }
            return Img;
        }

        UWidget* BuildSpec(UWidgetBlueprint* Wbp, UPanelWidget* Parent, const FWidgetSpec& Spec)
        {
            switch (Spec.Type)
            {
                case EWidgetType::Canvas: return BuildCanvas(Wbp, Parent, Spec);
                case EWidgetType::Image:  return BuildImage(Wbp, Parent, Spec);
                case EWidgetType::Skip:   return nullptr;
                default:                  return nullptr;  // other types added in tasks 12-14
            }
        }
    }

    UWidgetBlueprint* FUmgBuilder::Build(const FUmgBuildContext& Ctx, const FWidgetSpec& Root)
    {
        UWidgetBlueprint* Wbp = GetOrCreateWbp(Ctx.PackagePath, Ctx.WbpName);
        if (!Wbp) return nullptr;

        // v1: clear and rebuild from scratch. Slot positions / event bindings on
        // user-edited nodes are NOT preserved. Reimport idempotence (counts/names
        // stable across runs) is asserted in Task 18.
        Wbp->WidgetTree->RootWidget = nullptr;
        BuildSpec(Wbp, nullptr, Root);

        FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Wbp);
        Wbp->MarkPackageDirty();
        return Wbp;
    }
}

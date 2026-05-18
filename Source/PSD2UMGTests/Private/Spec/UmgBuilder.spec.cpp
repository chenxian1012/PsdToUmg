// NOTE: WidgetTree.h transitively pulls EngineTypes.h (which defines the
// engine's ::EBlendMode). PsdDocument.h declares PSD2UMG::EBlendMode, and
// when PsdReader.spec.cpp (compiled earlier in this unity TU) introduces
// `using namespace PSD2UMG;` at global scope, the two collide here.
// Workaround: don't bring PSD2UMG into the global namespace in this spec.
#include "Misc/AutomationTest.h"
#include "Builder/UmgBuilder.h"
#include "Blueprint/WidgetTree.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "WidgetBlueprint.h"

BEGIN_DEFINE_SPEC(FUmgBuilderSpec, "PSD2UMG.UmgBuilder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FUmgBuilderSpec)

void FUmgBuilderSpec::Define()
{
    Describe("canvas with one image", [this]()
    {
        It("creates WidgetBlueprint with CanvasPanel root and one Image child", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));

            PSD2UMG::FWidgetSpec Img;
            Img.Type = PSD2UMG::EWidgetType::Image; Img.WidgetName = TEXT("Logo");
            Img.Bounds = FBox2D(FVector2D(50, 50), FVector2D(562, 178));
            Img.Brush.TextureAssetName = TEXT("T_Logo");
            Img.Anchor = PSD2UMG::EAnchorPreset::TL;
            Root.Children.Add(MoveTemp(Img));

            PSD2UMG::FUmgBuildContext Ctx;
            Ctx.PackagePath = TEXT("/Game/PSD2UMG_UmgBuilderSpec");
            Ctx.WbpName     = TEXT("WBP_Spec_Simple");

            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("wbp", Wbp);

            UCanvasPanel* CanvasRoot = Cast<UCanvasPanel>(Wbp->WidgetTree->RootWidget);
            TestNotNull("canvas root", CanvasRoot);
            TestEqual("child count", CanvasRoot->GetChildrenCount(), 1);

            UImage* ImgWidget = Wbp->WidgetTree->FindWidget<UImage>(TEXT("Logo"));
            TestNotNull("image widget", ImgWidget);
        });
    });
}

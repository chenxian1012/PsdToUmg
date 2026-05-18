// NOTE: WidgetTree.h transitively pulls EngineTypes.h (which defines the
// engine's ::EBlendMode). PsdDocument.h declares PSD2UMG::EBlendMode, and
// when PsdReader.spec.cpp (compiled earlier in this unity TU) introduces
// `using namespace PSD2UMG;` at global scope, the two collide here.
// Workaround: don't bring PSD2UMG into the global namespace in this spec.
#include "Misc/AutomationTest.h"
#include "Builder/UmgBuilder.h"
#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/Image.h"
#include "Components/NamedSlot.h"
#include "Components/ProgressBar.h"
#include "Components/ScaleBox.h"
#include "Components/SizeBox.h"
#include "Components/TextBlock.h"
#include "Kismet2/KismetEditorUtilities.h"
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

    Describe("button with three states", [this]()
    {
        It("creates one UButton and accepts Normal/Hovered/Pressed brush specs without crashing", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas;
            Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));

            PSD2UMG::FWidgetSpec Btn;
            Btn.Type = PSD2UMG::EWidgetType::Button;
            Btn.WidgetName = TEXT("Play");
            Btn.Bounds = FBox2D(FVector2D(100, 100), FVector2D(300, 160));
            Btn.Anchor = PSD2UMG::EAnchorPreset::TL;
            Btn.ButtonStates.Add(PSD2UMG::EButtonState::Normal,  PSD2UMG::FSlateBrushSpec{});
            Btn.ButtonStates.Add(PSD2UMG::EButtonState::Hovered, PSD2UMG::FSlateBrushSpec{});
            Btn.ButtonStates.Add(PSD2UMG::EButtonState::Pressed, PSD2UMG::FSlateBrushSpec{});
            Root.Children.Add(MoveTemp(Btn));

            PSD2UMG::FUmgBuildContext Ctx;
            Ctx.PackagePath = TEXT("/Game/PSD2UMG_UmgBuilderSpec_Button");
            Ctx.WbpName     = TEXT("WBP_Spec_Button");

            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("wbp", Wbp);

            UButton* B = Wbp->WidgetTree->FindWidget<UButton>(TEXT("Play"));
            TestNotNull("button widget", B);
        });
    });

    Describe("progress bar", [this]()
    {
        It("creates a UProgressBar", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec PB; PB.Type = PSD2UMG::EWidgetType::ProgressBar;
            PB.WidgetName = TEXT("HpBar"); PB.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(200, 20));
            Root.Children.Add(MoveTemp(PB));
            PSD2UMG::FUmgBuildContext Ctx { TEXT("/Game/PSD2UMG_UmgBuilderSpec_PB"), TEXT("WBP_Spec_PB") };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("pb", Wbp->WidgetTree->FindWidget<UProgressBar>(TEXT("HpBar")));
        });
    });

    Describe("text block", [this]()
    {
        It("creates a UTextBlock with text content", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec T; T.Type = PSD2UMG::EWidgetType::Text; T.WidgetName = TEXT("Title");
            T.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(400, 40));
            T.TextStyle.Text = TEXT("Hello"); T.TextStyle.FontSizePx = 24;
            Root.Children.Add(MoveTemp(T));
            PSD2UMG::FUmgBuildContext Ctx { TEXT("/Game/PSD2UMG_UmgBuilderSpec_T"), TEXT("WBP_Spec_T") };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            UTextBlock* Tb = Wbp->WidgetTree->FindWidget<UTextBlock>(TEXT("Title"));
            TestNotNull("text", Tb);
            TestEqual("text content", Tb->GetText().ToString(), FString(TEXT("Hello")));
        });
    });

    Describe("named slot", [this]()
    {
        It("creates a UNamedSlot", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec NS; NS.Type = PSD2UMG::EWidgetType::NamedSlot;
            NS.WidgetName = TEXT("HeaderSlot"); NS.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(600, 80));
            Root.Children.Add(MoveTemp(NS));
            PSD2UMG::FUmgBuildContext Ctx { TEXT("/Game/PSD2UMG_UmgBuilderSpec_NS"), TEXT("WBP_Spec_NS") };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("ns", Wbp->WidgetTree->FindWidget<UNamedSlot>(TEXT("HeaderSlot")));
        });
    });

    Describe("size box", [this]()
    {
        It("creates a USizeBox with width/height overrides", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec SB; SB.Type = PSD2UMG::EWidgetType::SizeBox;
            SB.WidgetName = TEXT("Fixed"); SB.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(200, 80));
            Root.Children.Add(MoveTemp(SB));
            PSD2UMG::FUmgBuildContext Ctx { TEXT("/Game/PSD2UMG_UmgBuilderSpec_SZ"), TEXT("WBP_Spec_SZ") };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("sb", Wbp->WidgetTree->FindWidget<USizeBox>(TEXT("Fixed")));
        });
    });

    Describe("scale box", [this]()
    {
        It("creates a UScaleBox", [this]()
        {
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas; Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec SB; SB.Type = PSD2UMG::EWidgetType::ScaleBox;
            SB.WidgetName = TEXT("Scale"); SB.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(400, 300));
            Root.Children.Add(MoveTemp(SB));
            PSD2UMG::FUmgBuildContext Ctx { TEXT("/Game/PSD2UMG_UmgBuilderSpec_SC"), TEXT("WBP_Spec_SC") };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(Ctx, Root);
            TestNotNull("sc", Wbp->WidgetTree->FindWidget<UScaleBox>(TEXT("Scale")));
        });
    });

    Describe("smart-object subwidget (LinkedPsd)", [this]()
    {
        It("creates a UUserWidget referencing the child WBP", [this]()
        {
            // 1. Pre-create a child WBP at the conventional sibling path.
            //    Spec.SubWidgetAssetName = "WBP_Avatar"  →  loaded from
            //    /Game/PSD2UMG_UmgBuilderSpec_Sub/Avatar/WBP_Avatar.WBP_Avatar
            PSD2UMG::FWidgetSpec ChildRoot;
            ChildRoot.Type = PSD2UMG::EWidgetType::Canvas;
            ChildRoot.WidgetName = TEXT("Root");
            ChildRoot.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(256, 256));
            PSD2UMG::FUmgBuildContext ChildCtx
            {
                TEXT("/Game/PSD2UMG_UmgBuilderSpec_Sub/Avatar"),
                TEXT("WBP_Avatar")
            };
            UWidgetBlueprint* ChildWbp = PSD2UMG::FUmgBuilder::Build(ChildCtx, ChildRoot);
            TestNotNull("child wbp", ChildWbp);
            // For the child's GeneratedClass to exist, we need to compile it.
            // FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified does NOT
            // compile by itself, so explicitly request a recompile.
            FKismetEditorUtilities::CompileBlueprint(ChildWbp);
            TestNotNull("child generated class", ChildWbp->GeneratedClass.Get());

            // 2. Build the parent that references it via SubWidgetAssetName.
            PSD2UMG::FWidgetSpec Root;
            Root.Type = PSD2UMG::EWidgetType::Canvas;
            Root.WidgetName = TEXT("Root");
            Root.Bounds = FBox2D(FVector2D::ZeroVector, FVector2D(1920, 1080));
            PSD2UMG::FWidgetSpec Sub;
            Sub.Type = PSD2UMG::EWidgetType::SubWidget;
            Sub.WidgetName = TEXT("AvatarSlot");
            Sub.SubWidgetAssetName = TEXT("WBP_Avatar");
            Sub.Bounds = FBox2D(FVector2D(10, 10), FVector2D(266, 266));
            Root.Children.Add(MoveTemp(Sub));

            PSD2UMG::FUmgBuildContext ParentCtx
            {
                TEXT("/Game/PSD2UMG_UmgBuilderSpec_Sub/Parent"),
                TEXT("WBP_Parent")
            };
            UWidgetBlueprint* Wbp = PSD2UMG::FUmgBuilder::Build(ParentCtx, Root);
            TestNotNull("parent wbp", Wbp);

            UUserWidget* W = Wbp->WidgetTree->FindWidget<UUserWidget>(TEXT("AvatarSlot"));
            TestNotNull("subwidget present", W);
        });
    });
}

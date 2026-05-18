#include "Misc/AutomationTest.h"
#include "TestUtils.h"
#include "Importer/PsdReader.h"
#include "Schema/PsdSchemaResolver.h"
#include "Schema/PsdSidecarLoader.h"

#include "Misc/FileHelper.h"

using namespace PSD2UMG;

namespace
{
    bool ParseSample(const FString& Name, FPsdDocument& OutDoc)
    {
        TArray<uint8> Bytes;
        if (!FFileHelper::LoadFileToArray(Bytes, *PSD2UMGTest::GetSamplePsdPath(Name)))
            return false;
        FString Err;
        return FPsdReader::Read(Bytes, OutDoc, Err);
    }
}

BEGIN_DEFINE_SPEC(FPsdSchemaResolverSpec, "PSD2UMG.SchemaResolver",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPsdSchemaResolverSpec)

void FPsdSchemaResolverSpec::Define()
{
    Describe("Buttons.psd", [this]()
    {
        It("merges 3 button-state layers into one Button widget", [this]()
        {
            FPsdDocument Doc;
            TestTrue("read", ParseSample(TEXT("Buttons"), Doc));

            FPsdSidecar Sidecar;
            FPsdSidecarLoader::TryLoad(PSD2UMGTest::GetSamplePsdPath(TEXT("Buttons")), Sidecar);

            FResolveContext Ctx;
            Ctx.Sidecar = Sidecar;
            Ctx.ProjectDefaultUseCommonUI = true;
            Ctx.PsdName = TEXT("Buttons");

            FWidgetSpec Root;
            TArray<FString> Warnings;
            FPsdSchemaResolver::Resolve(Doc, Ctx, Root, Warnings);

            TestEqual("root type",   (int)Root.Type, (int)EWidgetType::Canvas);
            TestEqual("one child (merged button)", Root.Children.Num(), 1);
            TestEqual("type",   (int)Root.Children[0].Type, (int)EWidgetType::Button);
            TestEqual("name",   Root.Children[0].WidgetName.ToString(), FString(TEXT("PlayBtn")));
            TestTrue ("commonui", Root.Children[0].bUseCommonUI);
            TestEqual("3 states aggregated", Root.Children[0].ButtonStates.Num(), 3);
            TestTrue ("normal state present",  Root.Children[0].ButtonStates.Contains(EButtonState::Normal));
            TestTrue ("hovered state present", Root.Children[0].ButtonStates.Contains(EButtonState::Hovered));
            TestTrue ("pressed state present", Root.Children[0].ButtonStates.Contains(EButtonState::Pressed));
            TestEqual("sidecar style override applied",
                Root.Children[0].StyleAssetRef.ToString(),
                FString(TEXT("/Game/UI/Styles/BSt_Primary.BSt_Primary")));
        });
    });

    Describe("LinkedPsd.psd", [this]()
    {
        It("emits a SubWidget child with SubWidgetAssetName=WBP_Avatar", [this]()
        {
            FPsdDocument Doc;
            TestTrue("read", ParseSample(TEXT("LinkedPsd"), Doc));

            FResolveContext Ctx;
            Ctx.ProjectDefaultUseCommonUI = true;
            Ctx.PsdName = TEXT("LinkedPsd");

            FWidgetSpec Root;
            TArray<FString> Warnings;
            FPsdSchemaResolver::Resolve(Doc, Ctx, Root, Warnings);

            TestEqual("one child", Root.Children.Num(), 1);
            TestEqual("subwidget type", (int)Root.Children[0].Type, (int)EWidgetType::SubWidget);
            TestEqual("WBP asset name",
                Root.Children[0].SubWidgetAssetName.ToString(),
                FString(TEXT("WBP_Avatar")));
        });
    });

    Describe("Simple.psd", [this]()
    {
        It("emits 3 Image children for 3 raster layers", [this]()
        {
            FPsdDocument Doc;
            TestTrue("read", ParseSample(TEXT("Simple"), Doc));

            FResolveContext Ctx;
            Ctx.ProjectDefaultUseCommonUI = true;
            Ctx.PsdName = TEXT("Simple");

            FWidgetSpec Root;
            TArray<FString> Warnings;
            FPsdSchemaResolver::Resolve(Doc, Ctx, Root, Warnings);

            TestEqual("3 children", Root.Children.Num(), 3);
            for (const FWidgetSpec& C : Root.Children)
            {
                TestEqual("image type", (int)C.Type, (int)EWidgetType::Image);
            }
        });
    });
}

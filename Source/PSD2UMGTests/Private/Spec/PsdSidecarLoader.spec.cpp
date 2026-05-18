#include "Misc/AutomationTest.h"
#include "TestUtils.h"
#include "Schema/PsdSidecarLoader.h"

using namespace PSD2UMG;

BEGIN_DEFINE_SPEC(FPsdSidecarLoaderSpec, "PSD2UMG.SidecarLoader",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPsdSidecarLoaderSpec)

void FPsdSidecarLoaderSpec::Define()
{
    Describe("Buttons.psd.json (exists)", [this]()
    {
        It("loads globals and per-layer overrides", [this]()
        {
            const FString Psd = PSD2UMGTest::GetSamplePsdPath(TEXT("Buttons"));
            FPsdSidecar S;
            const bool bOk = FPsdSidecarLoader::TryLoad(Psd, S);
            TestTrue ("ok", bOk);
            TestEqual("version",     S.Version, 1);
            TestEqual("designDpi",   S.Globals.DesignDpi, 1920);
            TestTrue ("useCommonUI", S.Globals.bUseCommonUI);
            TestTrue ("PlayBtn present", S.PerLayer.Contains(TEXT("PlayBtn")));
            TestEqual("commonButtonStyle",
                S.PerLayer[TEXT("PlayBtn")].CommonButtonStyle.ToString(),
                FString(TEXT("/Game/UI/Styles/BSt_Primary.BSt_Primary")));
        });
    });

    Describe("Simple.psd.json (does not exist)", [this]()
    {
        It("returns false and leaves the struct empty", [this]()
        {
            const FString Psd = PSD2UMGTest::GetSamplePsdPath(TEXT("Simple"));
            FPsdSidecar S;
            const bool bOk = FPsdSidecarLoader::TryLoad(Psd, S);
            TestFalse("no file -> false", bOk);
            TestEqual("empty PerLayer", S.PerLayer.Num(), 0);
        });
    });
}

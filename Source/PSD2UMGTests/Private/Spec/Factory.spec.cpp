#include "Misc/AutomationTest.h"
#include "TestUtils.h"
#include "Importer/PSD2UMGFactory.h"
#include "Asset/PSD2UMGCache.h"
#include "Engine/Texture2D.h"
#include "WidgetBlueprint.h"

BEGIN_DEFINE_SPEC(FPSD2UMGFactorySpec, "PSD2UMG.Factory",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPSD2UMGFactorySpec)

void FPSD2UMGFactorySpec::Define()
{
    Describe("import Buttons.psd", [this]()
    {
        It("produces WBP_Buttons + textures + PsdCache asset", [this]()
        {
            const FString Psd = PSD2UMGTest::GetSamplePsdPath(TEXT("Buttons"));
            UPSD2UMGCache* Cache = UPSD2UMGFactory::ImportFromFile(
                Psd, TEXT("/Game/PSD2UMG_FactorySpec/Buttons"));
            TestNotNull("cache", Cache);

            UWidgetBlueprint* Wbp = LoadObject<UWidgetBlueprint>(
                nullptr,
                TEXT("/Game/PSD2UMG_FactorySpec/Buttons/WBP_Buttons.WBP_Buttons"));
            TestNotNull("wbp", Wbp);
        });
    });

    Describe("import LinkedPsd.psd (recurses into Avatar.psd)", [this]()
    {
        It("produces both WBP_LinkedPsd and sibling WBP_Avatar", [this]()
        {
            const FString Psd = PSD2UMGTest::GetSamplePsdPath(TEXT("LinkedPsd"));
            UPSD2UMGCache* Cache = UPSD2UMGFactory::ImportFromFile(
                Psd, TEXT("/Game/PSD2UMG_FactorySpec/LinkedPsd"));
            TestNotNull("cache", Cache);

            UWidgetBlueprint* Parent = LoadObject<UWidgetBlueprint>(
                nullptr,
                TEXT("/Game/PSD2UMG_FactorySpec/LinkedPsd/WBP_LinkedPsd.WBP_LinkedPsd"));
            TestNotNull("parent wbp", Parent);

            UWidgetBlueprint* Child = LoadObject<UWidgetBlueprint>(
                nullptr,
                TEXT("/Game/PSD2UMG_FactorySpec/Avatar/WBP_Avatar.WBP_Avatar"));
            TestNotNull("child wbp (sibling location)", Child);
        });
    });
}

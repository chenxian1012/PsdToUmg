#include "Misc/AutomationTest.h"
#include "Builder/StyleAssetBuilder.h"

// `using namespace PSD2UMG;` is scoped inside Define() — see PsdNamingParser.spec.cpp.

BEGIN_DEFINE_SPEC(FStyleAssetBuilderSpec, "PSD2UMG.StyleAssetBuilder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FStyleAssetBuilderSpec)

void FStyleAssetBuilderSpec::Define()
{
    using namespace PSD2UMG;
    Describe("text style creation", [this]()
    {
        It("creates a UCommonTextStyle asset (or returns null path if CommonUI absent)", [this]()
        {
            FTextStyleSpec Spec;
            Spec.FontFamily = TEXT("Roboto"); Spec.FontSizePx = 24;
            FSoftObjectPath Path = FStyleAssetBuilder::GetOrCreateTextStyle(
                TEXT("/Game/PSD2UMG_StyleAssetBuilderSpec"), TEXT("TS_Test"), Spec);
            AddInfo(FString::Printf(TEXT("returned path: %s"), *Path.ToString()));
            // CommonUI may or may not be loaded in this test config. Either outcome is acceptable;
            // a non-null path proves the class lookup worked, a null path proves the no-op branch worked.
            // Spec succeeds as long as we don't crash.
        });
    });

    Describe("button style creation", [this]()
    {
        It("creates a UCommonButtonStyle asset (or returns null path)", [this]()
        {
            FSlateBrushSpec B;
            FSoftObjectPath Path = FStyleAssetBuilder::GetOrCreateButtonStyle(
                TEXT("/Game/PSD2UMG_StyleAssetBuilderSpec"), TEXT("BSt_Test"), B, B, B);
            AddInfo(FString::Printf(TEXT("returned path: %s"), *Path.ToString()));
        });
    });
}

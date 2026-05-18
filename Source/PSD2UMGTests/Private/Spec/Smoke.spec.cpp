#include "Misc/AutomationTest.h"
#include "TestUtils.h"
#include "Misc/Paths.h"
#include "HAL/PlatformFileManager.h"
#include "GenericPlatform/GenericPlatformFile.h"

BEGIN_DEFINE_SPEC(FPSD2UMGSmokeSpec, "PSD2UMG.Smoke",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPSD2UMGSmokeSpec)

void FPSD2UMGSmokeSpec::Define()
{
    Describe("sample directory layout", [this]()
    {
        It("contains all six sample PSDs and their expected.json files", [this]()
        {
            IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
            for (const TCHAR* Name : { TEXT("Simple"), TEXT("Nested"), TEXT("Buttons"),
                                       TEXT("NineSlice"), TEXT("LinkedPsd"), TEXT("Avatar") })
            {
                const FString Psd = PSD2UMGTest::GetSamplePsdPath(Name);
                const FString Json = FPaths::Combine(PSD2UMGTest::GetSamplesDir(),
                                                     FString(Name) + TEXT(".expected.json"));
                TestTrue(FString::Printf(TEXT("psd exists: %s"), *Psd),
                         PlatformFile.FileExists(*Psd));
                TestTrue(FString::Printf(TEXT("json exists: %s"), *Json),
                         PlatformFile.FileExists(*Json));
            }
        });
    });
}

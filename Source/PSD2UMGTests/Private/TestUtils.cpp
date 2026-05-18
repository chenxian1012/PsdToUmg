#include "TestUtils.h"

#include "Interfaces/IPluginManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace PSD2UMGTest
{
    FString GetSamplesDir()
    {
        const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin("PSD2UMG");
        check(Plugin.IsValid());
        return FPaths::Combine(Plugin->GetBaseDir(), TEXT("Tests"), TEXT("Sample"));
    }

    FString GetSamplePsdPath(const FString& SampleName)
    {
        return FPaths::Combine(GetSamplesDir(), SampleName + TEXT(".psd"));
    }

    TSharedPtr<FJsonObject> LoadExpectedJson(const FString& SampleName)
    {
        const FString Path = FPaths::Combine(GetSamplesDir(), SampleName + TEXT(".expected.json"));
        FString Raw;
        const bool bRead = FFileHelper::LoadFileToString(Raw, *Path);
        check(bRead);

        TSharedPtr<FJsonObject> Parsed;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
        const bool bOk = FJsonSerializer::Deserialize(Reader, Parsed);
        check(bOk && Parsed.IsValid());
        return Parsed;
    }
}

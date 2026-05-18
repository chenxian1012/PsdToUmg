#include "Misc/AutomationTest.h"
#include "TestUtils.h"
#include "Importer/PsdReader.h"
#include "Schema/PsdDocument.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Policies/CondensedJsonPrintPolicy.h"
#include "Policies/PrettyJsonPrintPolicy.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

// Set this to 1, build, run the spec, then set back to 0 and re-run.
// When 1, the spec writes the parsed FPsdDocument as the new snapshot.
// When 0, it compares parsed output to the existing snapshot.
#ifndef PSD2UMG_REGENERATE_SNAPSHOTS
#define PSD2UMG_REGENERATE_SNAPSHOTS 0
#endif

// NOTE: PSD2UMG::EBlendMode collides with the engine's ::EBlendMode (defined in
// EngineTypes.h, pulled in by UMG headers in other specs in this unity TU).
// We pull in only the specific names we need rather than `using namespace`.

namespace
{
    using PSD2UMG::FPsdDocument;
    using PSD2UMG::FPsdLayer;
    using PSD2UMG::ELayerKind;
    using PSD2UMG::FPsdReader;

    TSharedRef<FJsonObject> ToJson(const FPsdDocument& Doc)
    {
        TSharedRef<FJsonObject> Root = MakeShared<FJsonObject>();
        TArray<TSharedPtr<FJsonValue>> Size;
        Size.Add(MakeShared<FJsonValueNumber>(Doc.CanvasSize.X));
        Size.Add(MakeShared<FJsonValueNumber>(Doc.CanvasSize.Y));
        Root->SetArrayField(TEXT("canvasSize"), Size);
        Root->SetNumberField(TEXT("colorDepth"), Doc.ColorDepth);
        Root->SetNumberField(TEXT("layerCount"), Doc.Layers.Num());

        TArray<TSharedPtr<FJsonValue>> Layers;
        for (const FPsdLayer& L : Doc.Layers)
        {
            TSharedRef<FJsonObject> J = MakeShared<FJsonObject>();
            J->SetStringField(TEXT("name"), L.Name);
            const TCHAR* Kind = TEXT("Raster");
            switch (L.Kind)
            {
                case ELayerKind::Group:     Kind = TEXT("Group");     break;
                case ELayerKind::Text:      Kind = TEXT("Text");      break;
                case ELayerKind::LinkedPsd: Kind = TEXT("LinkedPsd"); break;
                default: break;
            }
            J->SetStringField(TEXT("kind"), Kind);
            TArray<TSharedPtr<FJsonValue>> B;
            B.Add(MakeShared<FJsonValueNumber>(L.Bounds.Min.X));
            B.Add(MakeShared<FJsonValueNumber>(L.Bounds.Min.Y));
            B.Add(MakeShared<FJsonValueNumber>(L.Bounds.Max.X));
            B.Add(MakeShared<FJsonValueNumber>(L.Bounds.Max.Y));
            J->SetArrayField(TEXT("bounds"), B);
            J->SetBoolField(TEXT("visible"), L.bVisible);
            J->SetNumberField(TEXT("opacity"), L.Opacity);
            if (L.Kind == ELayerKind::LinkedPsd)
            {
                J->SetStringField(TEXT("linkedRel"), L.LinkedRef.RelPath);
            }
            Layers.Add(MakeShared<FJsonValueObject>(J));
        }
        Root->SetArrayField(TEXT("layers"), Layers);
        return Root;
    }

    FString JsonToString(const TSharedRef<FJsonObject>& Obj)
    {
        FString Out;
        TSharedRef<TJsonWriter<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>> Writer =
            TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&Out);
        FJsonSerializer::Serialize(Obj, Writer);
        return Out;
    }
}

BEGIN_DEFINE_SPEC(FPsdReaderSpec, "PSD2UMG.PsdReader",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FPsdReaderSpec)

void FPsdReaderSpec::Define()
{
    using namespace PSD2UMG;
    for (const TCHAR* Sample : { TEXT("Simple"), TEXT("Nested"), TEXT("Buttons"),
                                  TEXT("NineSlice"), TEXT("LinkedPsd"), TEXT("Avatar") })
    {
        const FString SampleName(Sample);
        Describe(FString::Printf(TEXT("%s.psd"), *SampleName), [this, SampleName]()
        {
            It("parses without error and matches the expected.json snapshot", [this, SampleName]()
            {
                const FString PsdPath = PSD2UMGTest::GetSamplePsdPath(SampleName);
                TArray<uint8> Bytes;
                TestTrue(TEXT("load bytes"), FFileHelper::LoadFileToArray(Bytes, *PsdPath));

                FPsdDocument Doc; FString Err;
                const bool bOk = FPsdReader::Read(Bytes, Doc, Err);
                TestTrue(FString::Printf(TEXT("read ok: %s"), *Err), bOk);

                const FString SnapshotPath = FPaths::Combine(PSD2UMGTest::GetSamplesDir(),
                                                              SampleName + TEXT(".expected.json"));
                const FString Actual = JsonToString(ToJson(Doc));

#if PSD2UMG_REGENERATE_SNAPSHOTS
                FFileHelper::SaveStringToFile(Actual, *SnapshotPath);
                AddInfo(FString::Printf(TEXT("Snapshot written: %s"), *SnapshotPath));
#else
                FString Expected;
                TestTrue(TEXT("snapshot exists"),
                         FFileHelper::LoadFileToString(Expected, *SnapshotPath));
                if (!TestEqual(TEXT("snapshot matches actual"), Actual, Expected))
                {
                    AddError(FString::Printf(TEXT(
                        "FPsdReader output differs from %s.\n"
                        "If the diff is expected, set PSD2UMG_REGENERATE_SNAPSHOTS=1, "
                        "rebuild, run once, then set back to 0."),
                        *SnapshotPath));
                }
#endif
            });
        });
    }
}

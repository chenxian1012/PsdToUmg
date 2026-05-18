#include "Schema/PsdSidecarLoader.h"

#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"

namespace PSD2UMG
{
    bool FPsdSidecarLoader::TryLoad(const FString& PsdPath, FPsdSidecar& Out)
    {
        const FString JsonPath = PsdPath + TEXT(".json");
        FString Raw;
        if (!FFileHelper::LoadFileToString(Raw, *JsonPath))
        {
            return false;
        }

        TSharedPtr<FJsonObject> Root;
        const TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Raw);
        if (!FJsonSerializer::Deserialize(Reader, Root) || !Root.IsValid())
        {
            return false;
        }

        double V = 0;
        Root->TryGetNumberField(TEXT("version"), V);
        Out.Version = static_cast<int32>(V);

        const TSharedPtr<FJsonObject>* Globals;
        if (Root->TryGetObjectField(TEXT("globals"), Globals))
        {
            double Dpi = 0;
            if ((*Globals)->TryGetNumberField(TEXT("designDpi"), Dpi))
            {
                Out.Globals.DesignDpi = static_cast<int32>(Dpi);
            }
            (*Globals)->TryGetBoolField(TEXT("useCommonUI"), Out.Globals.bUseCommonUI);
        }

        const TSharedPtr<FJsonObject>* Layers;
        if (Root->TryGetObjectField(TEXT("layers"), Layers))
        {
            for (const auto& Pair : (*Layers)->Values)
            {
                if (!Pair.Value.IsValid() || Pair.Value->Type != EJson::Object) continue;
                FPsdSidecarLayerOverride Ov;
                const TSharedPtr<FJsonObject>& L = Pair.Value->AsObject();
                FString S;
                if (L->TryGetStringField(TEXT("textStyle"), S))         Ov.TextStyle         = FSoftObjectPath(S);
                if (L->TryGetStringField(TEXT("commonButtonStyle"), S)) Ov.CommonButtonStyle = FSoftObjectPath(S);
                if (L->TryGetStringField(TEXT("fontFace"), S))          Ov.FontFace          = FSoftObjectPath(S);
                Out.PerLayer.Add(Pair.Key, Ov);
            }
        }
        return true;
    }
}

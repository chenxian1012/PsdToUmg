#include "Importer/PsdReader.h"

THIRD_PARTY_INCLUDES_START
#include "Psd/Psd.h"
#include "Psd/PsdMallocAllocator.h"
#include "Psd/PsdNativeFile.h"
#include "Psd/PsdDocument.h"
#include "Psd/PsdParseDocument.h"
#include "Psd/PsdParseLayerMaskSection.h"
#include "Psd/PsdLayerMaskSection.h"
#include "Psd/PsdLayer.h"
#include "Psd/PsdLayerType.h"
#include "Psd/PsdChannelType.h"
#include "Psd/PsdChannel.h"
#include "Psd/PsdColorMode.h"
THIRD_PARTY_INCLUDES_END

#include "HAL/FileManager.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace PSD2UMG
{
    namespace
    {
        bool ExtractLinkedPsdTag(FString& Name, FString& OutRel)
        {
            const FString Tag = TEXT("#linkedpsd(");
            const int32 Start = Name.Find(Tag);
            if (Start == INDEX_NONE) return false;
            const int32 ArgStart = Start + Tag.Len();
            const int32 ArgEnd = Name.Find(TEXT(")"), ESearchCase::IgnoreCase,
                                            ESearchDir::FromStart, ArgStart);
            if (ArgEnd == INDEX_NONE) return false;
            OutRel = Name.Mid(ArgStart, ArgEnd - ArgStart);
            Name = Name.Left(Start) + Name.Mid(ArgEnd + 1);
            Name.TrimStartAndEndInline();
            return true;
        }

        FString LayerNameToString(const psd::Layer* L)
        {
            // utf16Name is a uint16_t* heap buffer (may be null). Prefer it when
            // available so non-ASCII names round-trip; fall back to the fixed-size
            // ASCII name field otherwise.
            if (L->utf16Name)
            {
                // Walk the buffer to find its length (null-terminated uint16_t array).
                int32 Len = 0;
                while (L->utf16Name[Len] != 0)
                {
                    ++Len;
                }
                if (Len > 0)
                {
                    // Reinterpret the uint16_t* as UTF-16 code units (UCS2CHAR).
                    const UCS2CHAR* Ucs2 = reinterpret_cast<const UCS2CHAR*>(L->utf16Name);
                    return FString(static_cast<int32>(Len), Ucs2);
                }
            }
            return FString(UTF8_TO_TCHAR(L->name.c_str()));
        }

        void ExtractRgba8(const psd::Layer* L, TArray<uint8>& OutRgba)
        {
            const int32 W = L->right - L->left;
            const int32 H = L->bottom - L->top;
            if (W <= 0 || H <= 0) return;

            const uint8_t* RGBA[4] = { nullptr, nullptr, nullptr, nullptr };
            for (unsigned int i = 0; i < L->channelCount; ++i)
            {
                const int16_t T = L->channels[i].type;
                if (T == static_cast<int16_t>(psd::channelType::R))
                    RGBA[0] = static_cast<const uint8_t*>(L->channels[i].data);
                else if (T == static_cast<int16_t>(psd::channelType::G))
                    RGBA[1] = static_cast<const uint8_t*>(L->channels[i].data);
                else if (T == static_cast<int16_t>(psd::channelType::B))
                    RGBA[2] = static_cast<const uint8_t*>(L->channels[i].data);
                else if (T == static_cast<int16_t>(psd::channelType::TRANSPARENCY_MASK))
                    RGBA[3] = static_cast<const uint8_t*>(L->channels[i].data);
            }

            OutRgba.SetNumUninitialized(W * H * 4);
            const int32 PixelCount = W * H;
            for (int32 i = 0; i < PixelCount; ++i)
            {
                OutRgba[i * 4 + 0] = RGBA[0] ? RGBA[0][i] : 0;
                OutRgba[i * 4 + 1] = RGBA[1] ? RGBA[1][i] : 0;
                OutRgba[i * 4 + 2] = RGBA[2] ? RGBA[2][i] : 0;
                OutRgba[i * 4 + 3] = RGBA[3] ? RGBA[3][i] : 255;
            }
        }

        ELayerKind ClassifyLayer(const psd::Layer* L, FString& InOutName, FString& OutLinkedRel)
        {
            if (ExtractLinkedPsdTag(InOutName, OutLinkedRel))
            {
                return ELayerKind::LinkedPsd;
            }
            // psd_sdk encodes layerType as a uint32 from layerType::Enum.
            if (L->type == psd::layerType::OPEN_FOLDER ||
                L->type == psd::layerType::CLOSED_FOLDER ||
                L->type == psd::layerType::SECTION_DIVIDER)
            {
                return ELayerKind::Group;
            }
            return ELayerKind::Raster;
        }
    }

    bool FPsdReader::Read(TArrayView<const uint8> PsdBytes, FPsdDocument& OutDoc, FString& OutError)
    {
        // psd_sdk takes a file path. Stage to a temp file.
        const FString TempRoot = FPaths::ProjectIntermediateDir() / TEXT("PSD2UMG");
        IFileManager::Get().MakeDirectory(*TempRoot, /*Tree=*/true);
        const FString Tmp = TempRoot / FString::Printf(TEXT("read_%p.psd"), PsdBytes.GetData());
        if (!FFileHelper::SaveArrayToFile(PsdBytes, *Tmp))
        {
            OutError = TEXT("PsdReader: cannot stage temp file");
            return false;
        }

        psd::MallocAllocator Alloc;
        psd::NativeFile File(&Alloc);
        if (!File.OpenRead(TCHAR_TO_WCHAR(*Tmp)))
        {
            OutError = TEXT("psd_sdk: failed to open file");
            IFileManager::Get().Delete(*Tmp);
            return false;
        }

        psd::Document* Doc = psd::CreateDocument(&File, &Alloc);
        if (!Doc)
        {
            OutError = TEXT("psd_sdk: not a valid PSD (PSB unsupported)");
            File.Close();
            IFileManager::Get().Delete(*Tmp);
            return false;
        }
        if (Doc->colorMode != static_cast<unsigned int>(psd::colorMode::RGB))
        {
            OutError = TEXT("psd_sdk: only RGB color mode supported in v1");
            psd::DestroyDocument(Doc, &Alloc);
            File.Close();
            IFileManager::Get().Delete(*Tmp);
            return false;
        }

        OutDoc.CanvasSize = FIntPoint(static_cast<int32>(Doc->width), static_cast<int32>(Doc->height));
        OutDoc.ColorDepth = static_cast<int32>(Doc->bitsPerChannel);

        psd::LayerMaskSection* Section = psd::ParseLayerMaskSection(Doc, &File, &Alloc);
        if (Section)
        {
            for (unsigned int i = 0; i < Section->layerCount; ++i)
            {
                psd::Layer* L = &Section->layers[i];
                FPsdLayer Out;
                Out.Name = LayerNameToString(L);
                FString LinkedRel;
                Out.Kind = ClassifyLayer(L, Out.Name, LinkedRel);
                Out.Bounds = FBox2D(FVector2D(L->left, L->top), FVector2D(L->right, L->bottom));
                Out.Opacity = L->opacity / 255.0f;
                Out.bVisible = L->isVisible;

                if (Out.Kind == ELayerKind::LinkedPsd)
                {
                    Out.LinkedRef.RelPath = LinkedRel;
                }
                else if (Out.Kind == ELayerKind::Raster)
                {
                    psd::ExtractLayer(Doc, &File, &Alloc, L);
                    ExtractRgba8(L, Out.Pixels);
                }
                // Tree reconstruction (group nesting via SECTION_DIVIDER markers) is
                // deferred to PsdSchemaResolver (Task 8) - keep PsdReader's output flat.
                OutDoc.Layers.Add(MoveTemp(Out));
            }
            psd::DestroyLayerMaskSection(Section, &Alloc);
        }

        psd::DestroyDocument(Doc, &Alloc);
        File.Close();
        IFileManager::Get().Delete(*Tmp);
        return true;
    }
}

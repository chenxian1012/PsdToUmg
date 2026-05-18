#include "Builder/TextureBuilder.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/Texture2D.h"
#include "UObject/Package.h"

namespace PSD2UMG
{
    UTexture2D* FTextureBuilder::GetOrCreate(const FTextureBuilderRequest& Req)
    {
        if (Req.Width <= 0 || Req.Height <= 0 ||
            Req.RgbaPixels.Num() != Req.Width * Req.Height * 4)
        {
            return nullptr;
        }

        UPackage* Package = CreatePackage(*Req.PackagePath);
        Package->FullyLoad();

        UTexture2D* Existing = FindObject<UTexture2D>(Package, *Req.AssetName);
        UTexture2D* Tex = Existing
            ? Existing
            : NewObject<UTexture2D>(Package, *Req.AssetName, RF_Public | RF_Standalone);

        Tex->Source.Init(Req.Width, Req.Height, 1, 1, ETextureSourceFormat::TSF_BGRA8);
        uint8* Dst = Tex->Source.LockMip(0);
        const uint8* Src = Req.RgbaPixels.GetData();
        const int32 PixelCount = Req.Width * Req.Height;
        for (int32 i = 0; i < PixelCount; ++i)
        {
            Dst[i * 4 + 0] = Src[i * 4 + 2]; // B
            Dst[i * 4 + 1] = Src[i * 4 + 1]; // G
            Dst[i * 4 + 2] = Src[i * 4 + 0]; // R
            Dst[i * 4 + 3] = Src[i * 4 + 3]; // A
        }
        Tex->Source.UnlockMip(0);

        Tex->CompressionSettings = TC_EditorIcon;
        Tex->SRGB = true;
        Tex->MipGenSettings = TMGS_NoMipmaps;
        Tex->LODGroup = TEXTUREGROUP_UI;
        Tex->UpdateResource();
        Tex->MarkPackageDirty();

        if (!Existing)
        {
            FAssetRegistryModule::AssetCreated(Tex);
        }
        return Tex;
    }
}

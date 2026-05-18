#include "Misc/AutomationTest.h"
#include "Builder/TextureBuilder.h"
#include "Engine/Texture2D.h"

// `using namespace PSD2UMG;` is scoped inside Define() — see PsdNamingParser.spec.cpp.

BEGIN_DEFINE_SPEC(FTextureBuilderSpec, "PSD2UMG.TextureBuilder",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
END_DEFINE_SPEC(FTextureBuilderSpec)

void FTextureBuilderSpec::Define()
{
    using namespace PSD2UMG;
    Describe("4x4 red square", [this]()
    {
        It("creates UTexture2D with TC_UI / sRGB / 4x4 BGRA8 source", [this]()
        {
            TArray<uint8> Rgba;
            Rgba.SetNumUninitialized(4 * 4 * 4);
            for (int32 i = 0; i < 16; ++i)
            {
                Rgba[i*4+0] = 255; Rgba[i*4+1] = 0; Rgba[i*4+2] = 0; Rgba[i*4+3] = 255;
            }

            FTextureBuilderRequest Req;
            Req.PackagePath = TEXT("/Game/PSD2UMG_TextureBuilderSpec");
            Req.AssetName   = TEXT("T_RedSquare");
            Req.Width  = 4; Req.Height = 4;
            Req.RgbaPixels = Rgba;

            UTexture2D* Tex = FTextureBuilder::GetOrCreate(Req);
            TestNotNull("texture",  Tex);
            TestEqual("width",      (int32)Tex->Source.GetSizeX(), 4);
            TestEqual("height",     (int32)Tex->Source.GetSizeY(), 4);
            TestTrue ("srgb",       Tex->SRGB);
            TestEqual("compression",(int32)Tex->CompressionSettings, (int32)TC_EditorIcon);
            TestEqual("mip",        (int32)Tex->MipGenSettings,      (int32)TMGS_NoMipmaps);
            TestEqual("lod group",  (int32)Tex->LODGroup,            (int32)TEXTUREGROUP_UI);

            // Verify the channel swap: source byte 0 should now be 0 (B), 2 should be 255 (R).
            uint8* Src = Tex->Source.LockMip(0);
            TestEqual("B[0]", (int32)Src[0], 0);
            TestEqual("G[0]", (int32)Src[1], 0);
            TestEqual("R[0]", (int32)Src[2], 255);
            TestEqual("A[0]", (int32)Src[3], 255);
            Tex->Source.UnlockMip(0);
        });
    });

    Describe("invalid request", [this]()
    {
        It("returns nullptr on size 0", [this]()
        {
            FTextureBuilderRequest Req;
            Req.PackagePath = TEXT("/Game/PSD2UMG_TextureBuilderSpec");
            Req.AssetName   = TEXT("T_Empty");
            UTexture2D* Tex = FTextureBuilder::GetOrCreate(Req);
            TestNull("nullptr", Tex);
        });

        It("returns nullptr on pixel-count mismatch", [this]()
        {
            FTextureBuilderRequest Req;
            Req.PackagePath = TEXT("/Game/PSD2UMG_TextureBuilderSpec");
            Req.AssetName   = TEXT("T_Mismatch");
            Req.Width = 4; Req.Height = 4;
            Req.RgbaPixels.SetNumZeroed(60);  // wrong: should be 64
            UTexture2D* Tex = FTextureBuilder::GetOrCreate(Req);
            TestNull("nullptr", Tex);
        });
    });
}

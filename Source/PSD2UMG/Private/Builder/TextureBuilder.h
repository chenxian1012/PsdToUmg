#pragma once
#include "CoreMinimal.h"

class UTexture2D;

namespace PSD2UMG
{
    struct FTextureBuilderRequest
    {
        FString PackagePath;     // /Game/.../T_Foo  (the asset's package path, no .uasset extension)
        FString AssetName;       // T_Foo
        int32   Width = 0;
        int32   Height = 0;
        TArray<uint8> RgbaPixels;
    };

    class FTextureBuilder
    {
    public:
        // Creates a new UTexture2D at PackagePath, or updates the existing one in place.
        // Pixels are RGBA8 row-major; will be swizzled to BGRA8 for the texture source.
        // Returns nullptr on invalid request (zero size, byte-count mismatch).
        static PSD2UMG_API UTexture2D* GetOrCreate(const FTextureBuilderRequest& Req);
    };
}

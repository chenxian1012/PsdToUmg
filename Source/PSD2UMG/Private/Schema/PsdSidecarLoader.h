#pragma once

#include "CoreMinimal.h"
#include "UObject/SoftObjectPath.h"

namespace PSD2UMG
{
    struct FPsdSidecarGlobals
    {
        int32 DesignDpi = 1920;
        bool  bUseCommonUI = true;
    };

    struct FPsdSidecarLayerOverride
    {
        FSoftObjectPath TextStyle;
        FSoftObjectPath CommonButtonStyle;
        FSoftObjectPath FontFace;
    };

    struct FPsdSidecar
    {
        int32 Version = 0;
        FPsdSidecarGlobals Globals;
        TMap<FString, FPsdSidecarLayerOverride> PerLayer;
    };

    class FPsdSidecarLoader
    {
    public:
        /**
         * Loads <PsdPath>.psd.json. Returns false if the file does not exist
         * or fails to parse. Caller may treat false as "no sidecar present".
         */
        static PSD2UMG_API bool TryLoad(const FString& PsdPath, FPsdSidecar& OutSidecar);
    };
}

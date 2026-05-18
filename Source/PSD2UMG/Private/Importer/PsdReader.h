#pragma once

#include "CoreMinimal.h"
#include "Schema/PsdDocument.h"

namespace PSD2UMG
{
    class FPsdReader
    {
    public:
        /**
         * Parse a .psd byte stream into FPsdDocument.
         * Returns false on hard failure with reason in OutError.
         * PSB (8BPB) and non-RGB color modes return false with a specific error.
         */
        static PSD2UMG_API bool Read(TArrayView<const uint8> PsdBytes, FPsdDocument& OutDocument, FString& OutError);
    };
}

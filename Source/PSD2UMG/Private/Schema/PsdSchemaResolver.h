#pragma once

#include "CoreMinimal.h"
#include "Schema/PsdDocument.h"
#include "Schema/PsdSidecarLoader.h"
#include "Schema/WidgetSpec.h"

namespace PSD2UMG
{
    struct FResolveContext
    {
        FPsdSidecar Sidecar;
        bool        ProjectDefaultUseCommonUI = true;
        FString     PsdName;                  // file basename without extension
        TSet<FString> VisitedLinkedPsdPaths;  // absolute paths visited so far (cycle detection)
    };

    class FPsdSchemaResolver
    {
    public:
        static PSD2UMG_API void Resolve(const FPsdDocument& Doc, FResolveContext& Ctx,
                                         FWidgetSpec& OutRoot, TArray<FString>& OutWarnings);
    };
}

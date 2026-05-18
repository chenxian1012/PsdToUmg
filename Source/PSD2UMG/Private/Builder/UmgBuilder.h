#pragma once
#include "CoreMinimal.h"
#include "Schema/WidgetSpec.h"

class UWidgetBlueprint;
class UPanelWidget;
class UWidget;

namespace PSD2UMG
{
    struct FUmgBuildContext
    {
        FString PackagePath;     // /Game/UI/PsdImport/<PsdName>
        FString WbpName;         // WBP_<PsdName>
    };

    class FUmgBuilder
    {
    public:
        // Builds (or rebuilds) a WidgetBlueprint from the spec tree.
        // For v1, RootWidget is rebuilt from scratch each call — Reimport idempotence
        // verified at Task 18 by asserting node count/names stable across runs.
        static PSD2UMG_API UWidgetBlueprint* Build(const FUmgBuildContext& Ctx, const FWidgetSpec& Root);
    };
}

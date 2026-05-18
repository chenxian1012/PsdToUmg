#include "Asset/AssetDefinition_PSD2UMG.h"
#include "Asset/PSD2UMGCache.h"

#define LOCTEXT_NAMESPACE "PSD2UMG"

FText UAssetDefinition_PSD2UMG::GetAssetDisplayName() const
{
    return LOCTEXT("PSD2UMGCache", "PSD2UMG Cache");
}

TSoftClassPtr<UObject> UAssetDefinition_PSD2UMG::GetAssetClass() const
{
    return UPSD2UMGCache::StaticClass();
}

FLinearColor UAssetDefinition_PSD2UMG::GetAssetColor() const
{
    return FLinearColor(0.1f, 0.6f, 0.9f);
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_PSD2UMG::GetAssetCategories() const
{
    static const FAssetCategoryPath Cats[] = { FAssetCategoryPath(LOCTEXT("UI", "User Interface")) };
    return Cats;
}

#undef LOCTEXT_NAMESPACE

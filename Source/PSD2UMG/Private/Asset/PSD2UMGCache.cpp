#include "Asset/PSD2UMGCache.h"

#if WITH_EDITORONLY_DATA

void UPSD2UMGCache::PostInitProperties()
{
    Super::PostInitProperties();
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        AssetImportData = NewObject<UAssetImportData>(this, TEXT("AssetImportData"));
    }
}

void UPSD2UMGCache::GetAssetRegistryTags(FAssetRegistryTagsContext Context) const
{
    if (AssetImportData)
    {
        Context.AddTag(FAssetRegistryTag(
            UObject::SourceFileTagName(),
            AssetImportData->GetSourceData().ToJson(),
            FAssetRegistryTag::TT_Hidden));
    }
    Super::GetAssetRegistryTags(Context);
}
#endif

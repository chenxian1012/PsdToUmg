
/**

Copyright 2018 swd. All rights reserved.

*/


#include "AssetTypeActions_PSD2UMGCache.h"
//#include "PSD2UMGPrivatePCH.h"

#include "PSD2UMGCache.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

UClass* FAssetTypeActions_PSD2UMGCache::GetSupportedClass() const
{
	return UPSD2UMGCache::StaticClass();
}

bool FAssetTypeActions_PSD2UMGCache::IsImportedAsset() const
{
	return true;
}

void FAssetTypeActions_PSD2UMGCache::GetResolvedSourceFilePaths(const TArray<UObject*>& TypeAssets, TArray<FString>& OutSourceFilePaths) const
{
	for (auto& Asset : TypeAssets)
	{
		const auto asset = CastChecked<UPSD2UMGCache>(Asset);
		if (asset->AssetImportData)
		{
			asset->AssetImportData->ExtractFilenames(OutSourceFilePaths);
		}
	}
}

void FAssetTypeActions_PSD2UMGCache::GetActions(
	const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

#undef LOCTEXT_NAMESPACE
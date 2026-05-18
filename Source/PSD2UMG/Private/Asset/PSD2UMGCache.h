#pragma once

#include "CoreMinimal.h"
#include "EditorFramework/AssetImportData.h"
#include "PSD2UMGCache.generated.h"

UCLASS()
class PSD2UMG_API UPSD2UMGCache : public UObject
{
    GENERATED_BODY()
public:
#if WITH_EDITORONLY_DATA
    UPROPERTY(VisibleAnywhere, Instanced, Category = "ImportSettings")
    TObjectPtr<UAssetImportData> AssetImportData;
#endif

    virtual void GetAssetRegistryTags(FAssetRegistryTagsContext Context) const override;
    virtual void PostInitProperties() override;
};

#pragma once

#include "CoreMinimal.h"
#include "AssetDefinitionDefault.h"
#include "AssetDefinition_PSD2UMG.generated.h"

UCLASS()
class UAssetDefinition_PSD2UMG : public UAssetDefinitionDefault
{
    GENERATED_BODY()
public:
    virtual FText GetAssetDisplayName() const override;
    virtual TSoftClassPtr<UObject> GetAssetClass() const override;
    virtual FLinearColor GetAssetColor() const override;
    virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
};

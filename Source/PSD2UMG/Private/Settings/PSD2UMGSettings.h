#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Engine/Texture.h"
#include "PSD2UMGSettings.generated.h"

UCLASS(config = Editor, defaultconfig, meta = (DisplayName = "PSD2UMG"))
class PSD2UMG_API UPSD2UMGSettings : public UDeveloperSettings
{
    GENERATED_BODY()
public:
    UPROPERTY(EditAnywhere, config, Category = "General",
              meta = (DisplayName = "Default to CommonUI widgets",
                      ToolTip = "If on, generated buttons/text default to CommonUI base classes."))
    bool bDefaultToCommonUI = true;

    UPROPERTY(EditAnywhere, config, Category = "General",
              meta = (DisplayName = "Default texture compression"))
    TEnumAsByte<TextureCompressionSettings> DefaultCompression = TC_EditorIcon;

    UPROPERTY(EditAnywhere, config, Category = "General",
              meta = (DisplayName = "Generated asset root path"))
    FString GeneratedRootPath = TEXT("/Game/UI/PsdImport");

    virtual FName GetCategoryName() const override { return TEXT("Plugins"); }
};

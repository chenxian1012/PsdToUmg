#pragma once

#include "CoreMinimal.h"
#include "EditorReimportHandler.h"
#include "Factories/Factory.h"
#include "PSD2UMGFactory.generated.h"

class UPSD2UMGCache;

UCLASS()
class PSD2UMG_API UPSD2UMGFactory : public UFactory, public FReimportHandler
{
    GENERATED_BODY()
public:
    UPSD2UMGFactory(const FObjectInitializer& ObjectInitializer);

    /** Convenience entry point used by Automation tests. */
    static UPSD2UMGCache* ImportFromFile(const FString& PsdAbsolutePath, const FString& OutputPackagePath);

    virtual UObject* FactoryCreateBinary(
        UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags,
        UObject* Context, const TCHAR* Type, const uint8*& Buffer, const uint8* BufferEnd,
        FFeedbackContext* Warn) override;

    virtual bool DoesSupportClass(UClass* Class) override;
    virtual UClass* ResolveSupportedClass() override;

    virtual bool CanReimport(UObject* Obj, TArray<FString>& OutFilenames) override;
    virtual void SetReimportPaths(UObject* Obj, const TArray<FString>& NewReimportPaths) override;
    virtual EReimportResult::Type Reimport(UObject* Obj) override;
};

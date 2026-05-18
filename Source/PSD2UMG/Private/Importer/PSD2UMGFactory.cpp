#include "Importer/PSD2UMGFactory.h"

#include "Asset/PSD2UMGCache.h"
#include "Builder/TextureBuilder.h"
#include "Builder/UmgBuilder.h"
#include "Importer/PsdReader.h"
#include "Schema/PsdDocument.h"
#include "Schema/PsdNamingParser.h"
#include "Schema/PsdSchemaResolver.h"
#include "Schema/PsdSidecarLoader.h"
#include "Settings/PSD2UMGSettings.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "EditorFramework/AssetImportData.h"
#include "HAL/FileManager.h"
#include "Logging/MessageLog.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#define LOCTEXT_NAMESPACE "PSD2UMG"

UPSD2UMGFactory::UPSD2UMGFactory(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SupportedClass = UPSD2UMGCache::StaticClass();
    bCreateNew = false;
    bEditorImport = true;
    bText = false;
    Formats.Add(TEXT("psd;Photoshop Document"));
}

bool UPSD2UMGFactory::DoesSupportClass(UClass* Class) { return Class == UPSD2UMGCache::StaticClass(); }
UClass* UPSD2UMGFactory::ResolveSupportedClass()       { return UPSD2UMGCache::StaticClass(); }

namespace
{
    using namespace PSD2UMG;

    /** Strip characters disallowed in UE long package names (#, spaces, etc). */
    FString SanitizeAssetName(const FString& In)
    {
        FString Out;
        Out.Reserve(In.Len());
        for (TCHAR C : In)
        {
            const bool bOk = FChar::IsAlnum(C) || C == TEXT('_') || C == TEXT('-');
            Out.AppendChar(bOk ? C : TEXT('_'));
        }
        return Out;
    }

    /**
     * Builds one WBP from PSD bytes located at PsdAbsolutePath. Recurses into any
     * #linkedpsd(...) layers' sibling .psd files. Returns the created WBP (or nullptr).
     */
    void BuildOneWbp(const TArray<uint8>& PsdBytes,
                     const FString& PsdAbsolutePath,
                     const FString& OutputPackagePath,
                     const FString& WbpName,
                     FResolveContext& Ctx,
                     FMessageLog& Log)
    {
        FPsdDocument Doc;
        FString Err;
        if (!FPsdReader::Read(PsdBytes, Doc, Err))
        {
            Log.Error(FText::Format(LOCTEXT("ReadFail", "PSD read failed: {0}"),
                                     FText::FromString(Err)));
            return;
        }

        // 1. Emit one UTexture2D per raster layer.
        for (const FPsdLayer& L : Doc.Layers)
        {
            if (L.Kind != ELayerKind::Raster || L.Pixels.Num() == 0) continue;
            FTextureBuilderRequest TexReq;
            TexReq.AssetName   = FString(TEXT("T_")) + SanitizeAssetName(L.Name);
            TexReq.PackagePath = OutputPackagePath / TexReq.AssetName;
            TexReq.Width       = FMath::RoundToInt(L.Bounds.GetSize().X);
            TexReq.Height      = FMath::RoundToInt(L.Bounds.GetSize().Y);
            TexReq.RgbaPixels  = L.Pixels;
            FTextureBuilder::GetOrCreate(TexReq);
        }

        // 2. Recurse into linked sibling PSDs FIRST (so SubWidget builder can load child WBP).
        for (const FPsdLayer& L : Doc.Layers)
        {
            if (L.Kind != ELayerKind::LinkedPsd) continue;
            if (L.LinkedRef.RelPath.IsEmpty())
            {
                Log.Warning(FText::Format(LOCTEXT("LinkEmpty",
                    "Linked PSD layer '{0}' has empty relpath; skipping"),
                    FText::FromString(L.Name)));
                continue;
            }
            const FString ParentDir = FPaths::GetPath(PsdAbsolutePath);
            const FString AbsPath = FPaths::ConvertRelativePathToFull(
                ParentDir / L.LinkedRef.RelPath);

            if (Ctx.VisitedLinkedPsdPaths.Contains(AbsPath))
            {
                Log.Warning(FText::Format(LOCTEXT("LinkCycle",
                    "Linked PSD cycle on layer '{0}' ({1}); skipping"),
                    FText::FromString(L.Name), FText::FromString(AbsPath)));
                continue;
            }
            Ctx.VisitedLinkedPsdPaths.Add(AbsPath);

            TArray<uint8> SubBytes;
            if (!FFileHelper::LoadFileToArray(SubBytes, *AbsPath))
            {
                Log.Error(FText::Format(LOCTEXT("LinkMissing",
                    "Linked PSD not found: {0}"), FText::FromString(AbsPath)));
                continue;
            }
            const FString SubName        = FPaths::GetBaseFilename(AbsPath);
            const FString ImportRoot     = FPaths::GetPath(OutputPackagePath);
            const FString SubPackagePath = ImportRoot / SubName;
            BuildOneWbp(SubBytes, AbsPath, SubPackagePath,
                         FString(TEXT("WBP_")) + SubName, Ctx, Log);
        }

        // 3. Resolve schema and build WBP.
        FWidgetSpec Root;
        TArray<FString> Warnings;
        FPsdSchemaResolver::Resolve(Doc, Ctx, Root, Warnings);
        for (const FString& W : Warnings)
        {
            Log.Warning(FText::FromString(W));
        }

        FUmgBuildContext UCtx;
        UCtx.PackagePath = OutputPackagePath;
        UCtx.WbpName     = WbpName;
        FUmgBuilder::Build(UCtx, Root);
    }
}

UPSD2UMGCache* UPSD2UMGFactory::ImportFromFile(const FString& PsdAbsolutePath,
                                                const FString& OutPackagePath)
{
    FMessageLog Log("PSD2UMG");

    TArray<uint8> Bytes;
    if (!FFileHelper::LoadFileToArray(Bytes, *PsdAbsolutePath))
    {
        Log.Error(FText::Format(LOCTEXT("BadFile", "Cannot read {0}"),
                                 FText::FromString(PsdAbsolutePath)));
        return nullptr;
    }

    PSD2UMG::FResolveContext Ctx;
    PSD2UMG::FPsdSidecarLoader::TryLoad(PsdAbsolutePath, Ctx.Sidecar);

    const UPSD2UMGSettings* Settings = GetDefault<UPSD2UMGSettings>();
    Ctx.ProjectDefaultUseCommonUI = Settings ? Settings->bDefaultToCommonUI : true;
    Ctx.PsdName = FPaths::GetBaseFilename(PsdAbsolutePath);
    Ctx.VisitedLinkedPsdPaths.Add(FPaths::ConvertRelativePathToFull(PsdAbsolutePath));

    BuildOneWbp(Bytes, PsdAbsolutePath, OutPackagePath,
                 FString(TEXT("WBP_")) + Ctx.PsdName, Ctx, Log);

    // Create the Cache asset as Reimport handle.
    const FString CachePackageName = OutPackagePath / (FString(TEXT("PsdCache_")) + Ctx.PsdName);
    UPackage* Pkg = CreatePackage(*CachePackageName);
    UPSD2UMGCache* Cache = NewObject<UPSD2UMGCache>(
        Pkg, *(FString(TEXT("PsdCache_")) + Ctx.PsdName),
        RF_Public | RF_Standalone);
    if (Cache->AssetImportData)
    {
        Cache->AssetImportData->Update(PsdAbsolutePath);
    }
    FAssetRegistryModule::AssetCreated(Cache);
    Cache->MarkPackageDirty();
    return Cache;
}

UObject* UPSD2UMGFactory::FactoryCreateBinary(
    UClass*, UObject* InParent, FName InName, EObjectFlags,
    UObject*, const TCHAR*, const uint8*& Buffer, const uint8* BufferEnd, FFeedbackContext*)
{
    const FString OutPackagePath = InParent->GetOutermost()->GetName();

    // Persist incoming bytes so ImportFromFile can resolve sibling sidecars + linked PSDs.
    // We use CurrentFilename if available (the editor sets it when importing from disk);
    // otherwise we stage to a temp file.
    FString PsdAbsolutePath;
    if (!CurrentFilename.IsEmpty())
    {
        PsdAbsolutePath = CurrentFilename;
    }
    else
    {
        const FString TempDir = FPaths::ProjectIntermediateDir() / TEXT("PSD2UMG");
        IFileManager::Get().MakeDirectory(*TempDir, true);
        PsdAbsolutePath = TempDir / (InName.ToString() + TEXT(".psd"));
        TArray<uint8> Bytes(Buffer, BufferEnd - Buffer);
        FFileHelper::SaveArrayToFile(Bytes, *PsdAbsolutePath);
    }
    return ImportFromFile(PsdAbsolutePath, OutPackagePath);
}

bool UPSD2UMGFactory::CanReimport(UObject* Obj, TArray<FString>& OutFilenames)
{
    if (UPSD2UMGCache* Cache = Cast<UPSD2UMGCache>(Obj))
    {
        if (Cache->AssetImportData)
        {
            Cache->AssetImportData->ExtractFilenames(OutFilenames);
            return true;
        }
    }
    return false;
}

void UPSD2UMGFactory::SetReimportPaths(UObject* Obj, const TArray<FString>& Paths)
{
    if (UPSD2UMGCache* Cache = Cast<UPSD2UMGCache>(Obj))
    {
        if (Cache->AssetImportData && Paths.Num() == 1)
        {
            Cache->AssetImportData->UpdateFilenameOnly(Paths[0]);
        }
    }
}

EReimportResult::Type UPSD2UMGFactory::Reimport(UObject* Obj)
{
    UPSD2UMGCache* Cache = Cast<UPSD2UMGCache>(Obj);
    if (!Cache || !Cache->AssetImportData) return EReimportResult::Failed;
    const FString PsdPath = Cache->AssetImportData->GetFirstFilename();
    if (PsdPath.IsEmpty() || !FPaths::FileExists(PsdPath)) return EReimportResult::Failed;

    // Reimport reuses the same output package path as the original import.
    const FString OutPackagePath = FPaths::GetPath(Cache->GetOuter()->GetName());
    return ImportFromFile(PsdPath, OutPackagePath) != nullptr
        ? EReimportResult::Succeeded
        : EReimportResult::Failed;
}

#undef LOCTEXT_NAMESPACE

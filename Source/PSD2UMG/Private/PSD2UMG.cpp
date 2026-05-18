#include "IPSD2UMG.h"

#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"

// The installed engine Core.dll was built without __clang__ defined and
// therefore does not export the namespaced Windows::GetCurrentThreadId()
// symbol that MinimalWindowsApi.h's clang branch references. Our module
// IS compiled with clang (per BuildConfiguration.xml), so any TU that
// transitively instantiates an MT access detector (e.g. via DECLARE_DELEGATE_*
// macros in Factories/Factory.h) drags in this symbol. Alias the missing
// clang-mangled import to the plain kernel32 import that IS in Core's
// import table.
#if defined(_WIN64) && defined(__clang__)
    #pragma comment(linker, "/alternatename:__imp_?GetCurrentThreadId@Windows@@YAKXZ=__imp_GetCurrentThreadId")
#endif

#define LOCTEXT_NAMESPACE "PSD2UMG"

class FPSD2UMGModule : public IPSD2UMG
{
public:
    virtual void StartupModule() override
    {
        FMessageLogModule& MessageLogModule =
            FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        FMessageLogInitializationOptions Options;
        Options.bShowFilters = true;
        Options.bShowPages = true;
        Options.bAllowClear = true;
        MessageLogModule.RegisterLogListing("PSD2UMG", LOCTEXT("PSD2UMG", "PSD2UMG"), Options);
    }

    virtual void ShutdownModule() override
    {
        if (FModuleManager::Get().IsModuleLoaded("MessageLog"))
        {
            FMessageLogModule& MessageLogModule =
                FModuleManager::GetModuleChecked<FMessageLogModule>("MessageLog");
            MessageLogModule.UnregisterLogListing("PSD2UMG");
        }
    }
};

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FPSD2UMGModule, PSD2UMG)

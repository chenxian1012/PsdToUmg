#include "IPSD2UMG.h"

#include "Logging/MessageLog.h"
#include "MessageLogModule.h"
#include "Modules/ModuleManager.h"

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

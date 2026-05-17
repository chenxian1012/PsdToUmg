#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

class IPSD2UMG : public IModuleInterface
{
public:
    static IPSD2UMG& Get()
    {
        return FModuleManager::LoadModuleChecked<IPSD2UMG>("PSD2UMG");
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded("PSD2UMG");
    }
};

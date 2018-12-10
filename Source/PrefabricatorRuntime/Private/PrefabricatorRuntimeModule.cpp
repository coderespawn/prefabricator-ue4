//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorRuntimeModule.h"


class FPrefabricatorRuntime : public IPrefabricatorRuntime
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

IMPLEMENT_MODULE(FPrefabricatorRuntime, PrefabricatorRuntime)

void FPrefabricatorRuntime::StartupModule()
{
	
}


void FPrefabricatorRuntime::ShutdownModule()
{

}


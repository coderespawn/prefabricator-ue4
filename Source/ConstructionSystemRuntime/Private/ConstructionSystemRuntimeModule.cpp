//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemRuntimeModule.h"

#include "Utils/PrefabricatorService.h"

class FConstructionSystemRuntime : public IConstructionSystemRuntime
{
	/** IModuleInterface implementation */
	virtual void StartupModule() override {

	}
	virtual void ShutdownModule() override {

	}
};

IMPLEMENT_MODULE(FConstructionSystemRuntime, ConstructionSystemRuntime)


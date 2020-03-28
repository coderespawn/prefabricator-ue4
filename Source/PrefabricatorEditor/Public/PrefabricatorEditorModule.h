//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "PrefabEditorTypes.h"

#include "AssetTypeCategories.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "Toolkits/IToolkit.h"

/**
 * The public interface to this module
 */
class PREFABRICATOREDITOR_API IPrefabricatorEditorModule : public IModuleInterface
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IPrefabricatorEditorModule& Get()
	{
		return FModuleManager::LoadModuleChecked<IPrefabricatorEditorModule>("PrefabricatorEditor");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded("PrefabricatorEditor");
	}

	virtual EAssetTypeCategories::Type GetPrefabricatorAssetCategoryBit() const = 0;
	virtual FPrefabDetailsExtend& GetPrefabActorDetailsExtender() = 0;
	virtual void UpgradePrefabAssets() = 0;
};


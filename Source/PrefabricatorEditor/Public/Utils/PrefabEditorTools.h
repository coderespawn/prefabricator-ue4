//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class UPrefabricatorAsset;

class PREFABRICATOREDITOR_API FPrefabEditorTools {
public:
	static void ReloadPrefabsInLevel(UWorld* World, UPrefabricatorAsset* InAsset = nullptr);
};

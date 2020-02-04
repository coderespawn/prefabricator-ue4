//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "LevelEditor.h"

class PREFABRICATOREDITOR_API FMapChangeHook {
public:
	void Initialize();
	void Release();

private:
	void OnMapChanged(UWorld* World, EMapChangeType MapChangeType);

private:
	FDelegateHandle CallbackHandle;
};


//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class FPrefabricatorSelectionHook {
public:
	void Initialize();
	void Release();

private:
	void OnObjectSelected(UObject* Object);

private:
	FDelegateHandle CallbackHandle;
	TWeakObjectPtr<UObject> LastSelectedObject;
	bool bSelectionGuard = false;
};

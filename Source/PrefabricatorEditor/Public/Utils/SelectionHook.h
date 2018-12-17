//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

class FPrefabricatorSelectionHook {
public:
	void Initialize();
	void Release();

private:
	void OnObjectSelected(UObject* Object);
	void OnSelectNone();

private:
	FDelegateHandle CallbackHandle_SelectObject;
	FDelegateHandle CallbackHandle_SelectNone;
	TWeakObjectPtr<AActor> LastSelectedObject;
	bool bSelectionGuard = false;
};

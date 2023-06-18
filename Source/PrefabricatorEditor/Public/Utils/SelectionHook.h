//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

struct FPrefabricatorSelectionRequest {
	TWeakObjectPtr<AActor> Actor;
	bool bSelected;
};

class PREFABRICATOREDITOR_API FPrefabricatorSelectionHook : public FTickableEditorObject {
public:
	void Initialize();
	void Release();

	//~ Begin FTickableEditorObject Interface
	virtual void Tick(float DeltaTime) override;
	virtual bool IsTickable() const override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableEditorObject Interface

private:
	void OnObjectSelected(UObject* Object);
	void OnSelectNone();

private:
	FDelegateHandle CallbackHandle_SelectObject;
	FDelegateHandle CallbackHandle_SelectNone;
	TWeakObjectPtr<AActor> LastSelectedObject;
	bool bSelectionGuard = false;
	TQueue<FPrefabricatorSelectionRequest> SelectionRequests;
};


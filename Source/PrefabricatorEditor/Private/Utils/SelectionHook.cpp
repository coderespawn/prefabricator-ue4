//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/SelectionHook.h"

#include "Asset/PrefabricatorAssetUserData.h"
#include "Prefab/PrefabActor.h"

#include "Engine/Selection.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorSelectionHook, Log, All);

void FPrefabricatorSelectionHook::Initialize()
{
	CallbackHandle = USelection::SelectObjectEvent.AddRaw(this, &FPrefabricatorSelectionHook::OnObjectSelected);
}

void FPrefabricatorSelectionHook::Release()
{
	USelection::SelectObjectEvent.Remove(CallbackHandle);
}

namespace {
	bool IsInHierarchy(AActor* InActor, AActor* InPossibleParent) {
		if (!InActor || !InPossibleParent) {
			return false;
		}

		AActor* ParentActor = InActor;
		while (ParentActor) {
			if (ParentActor == InPossibleParent) {
				return true;
			}
			ParentActor = ParentActor->GetAttachParentActor();
		}
		return false;
	}

	APrefabActor* GetOutermostPrefab(AActor* InActor) {
		APrefabActor* OuterPrefab = Cast<APrefabActor>(InActor->GetAttachParentActor());
		if (!OuterPrefab) {
			return nullptr;
		}

		while (true) {
			APrefabActor* NextOuterPrefab = Cast<APrefabActor>(OuterPrefab->GetAttachParentActor());
			if (!NextOuterPrefab) {
				break;
			}
			OuterPrefab = NextOuterPrefab;
		}
		return OuterPrefab;
	}
}


void FPrefabricatorSelectionHook::OnObjectSelected(UObject* Object)
{
	if (bSelectionGuard) return;

	AActor* RequestedActor = Cast<AActor>(Object);
	if (!RequestedActor) {
		return;
	}

	AActor* CustomSelection = nullptr;

	bool bIsInHierarchy = LastSelectedObject.IsValid() && IsInHierarchy(RequestedActor, LastSelectedObject.Get());

	if (!bIsInHierarchy) {
		// Select the outermost prefab actor, if available
		APrefabActor* OutermostPrefab = GetOutermostPrefab(RequestedActor);
		if (OutermostPrefab) {
			CustomSelection = OutermostPrefab;
		}
	}
	else {
		// This object is in the previously selected prefab hierarchy
		// Try moving the selection one level up the prefab hierarchy chain
		// If we cannot do this, then we select the existing object
		APrefabActor* OuterPrefab = Cast<APrefabActor>(LastSelectedObject->GetAttachParentActor());
		if (OuterPrefab) {
			CustomSelection = OuterPrefab;
		}
	}

	if (CustomSelection) {
		bSelectionGuard = true;
		GEditor->SelectActor(RequestedActor, false, true);
		GEditor->SelectActor(CustomSelection, true, true);
		bSelectionGuard = false;

		LastSelectedObject = CustomSelection;
	}
	else {
		LastSelectedObject = RequestedActor;
	}

}


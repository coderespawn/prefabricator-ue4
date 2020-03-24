//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/SelectionHook.h"

#include "Asset/PrefabricatorAssetUserData.h"
#include "Prefab/PrefabActor.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorSelectionHook, Log, All);

void FPrefabricatorSelectionHook::Initialize()
{
	CallbackHandle_SelectObject = USelection::SelectObjectEvent.AddRaw(this, &FPrefabricatorSelectionHook::OnObjectSelected);
	CallbackHandle_SelectNone = USelection::SelectNoneEvent.AddRaw(this, &FPrefabricatorSelectionHook::OnSelectNone);
}

void FPrefabricatorSelectionHook::Release()
{
	USelection::SelectObjectEvent.Remove(CallbackHandle_SelectObject);
	USelection::SelectNoneEvent.Remove(CallbackHandle_SelectNone);
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

	USelection* ActorSelectionSet = GEditor->GetSelectedActors();
	if (ActorSelectionSet->Num() > 1) {
		//return;
	}
	if (ActorSelectionSet->Num() == 0) {
		LastSelectedObject = nullptr;
		return;
	}

	TArray<AActor*> SelectedActorObjects;
	ActorSelectionSet->GetSelectedObjects<AActor>(SelectedActorObjects);

	AActor* RequestedActor = Cast<AActor>(Object); // SelectedActorObjects[0];
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

void FPrefabricatorSelectionHook::OnSelectNone()
{
	LastSelectedObject = nullptr;
}


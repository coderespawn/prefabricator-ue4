//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Tools/SelectionHook.h"

#include "Asset/PrefabricatorAssetUserData.h"
#include "Prefab/PrefabActor.h"

#include "Engine/Selection.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorSelectionHook, Log, All);

void FPrefabricatorSelectionHook::Initialize()
{
	USelection::SelectObjectEvent.AddRaw(this, &FPrefabricatorSelectionHook::OnObjectSelected);
}

void FPrefabricatorSelectionHook::Release()
{
	USelection::SelectObjectEvent.Remove(CallbackHandle);
}

void FPrefabricatorSelectionHook::OnObjectSelected(UObject* Object)
{
	if (Object->IsA<APrefabActor>()) {

	}
	else {
		AActor* Actor = Cast<AActor>(Object);
		if (Actor && Actor->GetRootComponent()) {
			UPrefabricatorAssetUserData* PrefabUserData = Actor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
			if (PrefabUserData && PrefabUserData->PrefabActor.IsValid()) {
				// Make sure we are attached to this actor
				APrefabActor* PrefabActor = PrefabUserData->PrefabActor.Get();
				TArray<AActor*> AttachedPrefabActors;
				PrefabActor->GetAttachedActors(AttachedPrefabActors);
				if (AttachedPrefabActors.Contains(Actor)) {
					GEditor->SelectActor(Actor, false, true);
					GEditor->SelectActor(PrefabUserData->PrefabActor.Get(), true, true);
				}
			}
		}
	}
}


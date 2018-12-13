//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabEditorTools.h"
#include "EngineUtils.h"
#include "PrefabActor.h"
#include "PrefabActor.h"
#include "PrefabComponent.h"

void FPrefabEditorTools::ReloadPrefabsInLevel(UWorld* World, UPrefabricatorAsset* InAsset)
{
	for (TActorIterator<APrefabActor> It(World); It; ++It) {
		APrefabActor* PrefabActor = *It;
		if (PrefabActor && PrefabActor->PrefabComponent->PrefabAsset) {
			bool bShouldRefresh = true;
			if (InAsset) {
				UPrefabricatorAsset* ActorAsset = PrefabActor->PrefabComponent->PrefabAsset;
				bShouldRefresh = (InAsset == ActorAsset);
			}
			if (bShouldRefresh) {
				if (PrefabActor->IsPrefabOutdated()) {
					PrefabActor->LoadPrefab();
				}
			}
		}
	}
}

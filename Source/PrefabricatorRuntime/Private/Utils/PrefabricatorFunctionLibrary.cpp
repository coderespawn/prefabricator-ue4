//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorFunctionLibrary.h"
#include "Engine/Engine.h"
#include "PrefabricatorAsset.h"
#include "PrefabActor.h"
#include "PrefabComponent.h"

APrefabActor* UPrefabricatorBlueprintLibrary::SpawnPrefab(const UObject* WorldContextObject, UPrefabricatorAssetInterface* Prefab, const FTransform& Transform)
{
	APrefabActor* PrefabActor = nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World) {
		
		PrefabActor = World->SpawnActor<APrefabActor>(APrefabActor::StaticClass(), Transform);
		PrefabActor->PrefabComponent->PrefabAssetInterface = Prefab;
		PrefabActor->LoadPrefab();
	}
	return PrefabActor;
}

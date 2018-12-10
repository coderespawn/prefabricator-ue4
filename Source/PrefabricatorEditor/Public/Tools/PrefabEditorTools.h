//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class AActor;
class APrefabActor;

class PREFABRICATOREDITOR_API FPrefabEditorTools {
public:
	static bool CanCreatePrefab();
	static void CreatePrefab();
	static void CreatePrefabFromActors(const TArray<AActor*>& Actors);
	static void GetSelectedActors(TArray<AActor*>& OutActors);
	static void AddAssetUserData(AActor* InActor, APrefabActor* Prefab);
};

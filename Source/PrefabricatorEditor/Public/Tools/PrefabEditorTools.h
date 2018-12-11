//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class AActor;
class APrefabActor;
class UPrefabricatorAsset;
struct FPrefabricatorActorData;

class PREFABRICATOREDITOR_API FPrefabEditorTools {
public:
	static bool CanCreatePrefab();
	static void CreatePrefab();
	static void CreatePrefabFromActors(const TArray<AActor*>& Actors);
	static void AssignAssetUserData(AActor* InActor, APrefabActor* Prefab);
	static UPrefabricatorAsset* CreatePrefabAsset();

	static void SaveStateToPrefabAsset(APrefabActor* PrefabActor);
	static void LoadStateFromPrefabAsset(APrefabActor* PrefabActor);

	static void SaveStateToPrefabAsset(AActor* InActor, APrefabActor* PrefabActor, FPrefabricatorActorData& OutActorData);
	static void LoadStateFromPrefabAsset(AActor* InActor, APrefabActor* PrefabActor, const FPrefabricatorActorData& InActorData);

	static void GetActorChildren(AActor* InParent, TArray<AActor*>& OutChildren);


	static void ParentActors(AActor* ParentActor, AActor* ChildActor);
	static void SelectPrefabActor(AActor* PrefabActor);
	static void GetSelectedActors(TArray<AActor*>& OutActors);
	static int GetNumSelectedActors();
};

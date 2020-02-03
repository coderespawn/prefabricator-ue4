//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

class AActor;
class APrefabActor;
class UPrefabricatorAsset;
struct FPrefabricatorActorData;

struct PREFABRICATORRUNTIME_API FPrefabLoadSettings {
	bool bUnregisterComponentsBeforeLoading = true;
	bool bRandomizeNestedSeed = false;
	bool bSynchronousBuild = true;
	const FRandomStream* Random = nullptr;
};

class PREFABRICATORRUNTIME_API FPrefabTools {
public:
	static bool CanCreatePrefab();
	static void CreatePrefab();
	static void CreatePrefabFromActors(const TArray<AActor*>& Actors);
	static void AssignAssetUserData(AActor* InActor, const FGuid& InItemID, APrefabActor* Prefab);

	static void SaveStateToPrefabAsset(APrefabActor* PrefabActor);
	static void LoadStateFromPrefabAsset(APrefabActor* PrefabActor, const FPrefabLoadSettings& InSettings = FPrefabLoadSettings());

	static void SaveStateToPrefabAsset(AActor* InActor, APrefabActor* PrefabActor, FPrefabricatorActorData& OutActorData);
	static void LoadStateFromPrefabAsset(AActor* InActor, const FPrefabricatorActorData& InActorData, const FPrefabLoadSettings& InSettings);

	static void UnlinkAndDestroyPrefabActor(APrefabActor* PrefabActor);
	static void GetActorChildren(AActor* InParent, TArray<AActor*>& OutChildren);

	static FBox GetPrefabBounds(AActor* PrefabActor, bool bNonColliding = true);
	static bool ShouldIgnorePropertySerialization(const FName& PropertyName);
	static bool ShouldForcePropertySerialization(const FName& PropertyName);

	static void ParentActors(AActor* ParentActor, AActor* ChildActor);
	static void SelectPrefabActor(AActor* PrefabActor);
	static void GetSelectedActors(TArray<AActor*>& OutActors);
	static int GetNumSelectedActors();
	static UPrefabricatorAsset* CreatePrefabAsset();
	static int32 GetRandomSeed(const FRandomStream& Random);

	static void IterateChildrenRecursive(APrefabActor* Actor, TFunction<void(AActor*)> Visit);
};

class PREFABRICATORRUNTIME_API FPrefabVersionControl {
public:
	static void UpgradeToLatestVersion(UPrefabricatorAsset* Prefab);

private:
	static void UpgradeFromVersion_InitialVersion(UPrefabricatorAsset* Prefab);
	static void UpgradeFromVersion_AddedSoftReferences(UPrefabricatorAsset* Prefab);

};


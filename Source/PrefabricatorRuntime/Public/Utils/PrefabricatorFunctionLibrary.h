//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PrefabricatorFunctionLibrary.generated.h"

class UPrefabricatorAssetInterface;
class APrefabActor;
struct FWorldContext;

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	static APrefabActor* SpawnPrefab(const UObject* WorldContextObject, UPrefabricatorAssetInterface* Prefab, const FTransform& Transform, int32 Seed);

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	static void RandomizePrefab(APrefabActor* PrefabActor, const FRandomStream& InRandom);

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	static void GetAllAttachedActors(AActor* Prefab, TArray<AActor*>& AttachedActors);

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	static void SetPrefabAsset(APrefabActor* PrefabActor, UPrefabricatorAssetInterface* Prefab, bool bReloadPrefab);
};


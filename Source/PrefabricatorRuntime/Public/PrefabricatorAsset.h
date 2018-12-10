//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PrefabricatorAsset.generated.h"

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorFieldData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName FieldName;

	UPROPERTY()
	TArray<uint8> Data;
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorComponentData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FName ComponentName;

	UPROPERTY()
	TArray<FPrefabricatorFieldData> Fields;
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorActorData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform RelativeTransform;

	UPROPERTY()
	UClass* ActorClass;

	UPROPERTY()
	TArray<FPrefabricatorFieldData> Fields;

	UPROPERTY()
	TArray<FPrefabricatorComponentData> Components;
};


UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAsset : public UObject {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY()
	TArray<FPrefabricatorActorData> ActorData;
};


class PREFABRICATORRUNTIME_API FPrefabricatorAssetUtils {
public:
	static FVector FindPivot(const TArray<AActor*>& InActors);
	static EComponentMobility::Type FindMobility(const TArray<AActor*>& InActors);
	
};
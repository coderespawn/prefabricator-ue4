//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PrefabricatorAsset.generated.h"

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorPropertyBase : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PropertyName;
};

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorAtomProperty : public UPrefabricatorPropertyBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	FString ExportedValue;
};

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorArrayProperty : public UPrefabricatorPropertyBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FString> ExportedValues;
};

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorSetProperty : public UPrefabricatorPropertyBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FString> ExportedValues;
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorMapPropertyEntry {
	GENERATED_USTRUCT_BODY()

	USTRUCT()
	FString ExportedKey;

	USTRUCT()
	FString ExportedValue;
};

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorMapProperty : public UPrefabricatorPropertyBase {
	GENERATED_BODY()
public:
	UPROPERTY()
	TArray<FPrefabricatorMapPropertyEntry> ExportedEntries;
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorComponentData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform RelativeTransform;

	UPROPERTY()
	FString ComponentName;

	UPROPERTY()
	TArray<UPrefabricatorPropertyBase*> Properties;
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorActorData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FGuid PrefabItemID;

	UPROPERTY()
	FTransform RelativeTransform;

	UPROPERTY()
	FString ClassPath;

	UPROPERTY()
	TArray<UPrefabricatorPropertyBase*> Properties;

	UPROPERTY()
	TArray<FPrefabricatorComponentData> Components;
};

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAssetInterface : public UObject {
	GENERATED_BODY()
};

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAsset : public UPrefabricatorAssetInterface {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY()
	TArray<FPrefabricatorActorData> ActorData;

	UPROPERTY()
	TEnumAsByte<EComponentMobility::Type> PrefabMobility;

	// The ID that is regenerated on every update
	// This allows prefab actors to test against their own LastUpdateID and determine if a refresh is needed
	UPROPERTY()
	FGuid LastUpdateID;


	/** Information for thumbnail rendering */
	UPROPERTY(VisibleAnywhere, Instanced, AdvancedDisplay, Category = StaticMesh)
	class UThumbnailInfo* ThumbnailInfo;

};


class PREFABRICATORRUNTIME_API FPrefabricatorAssetUtils {
public:
	static FVector FindPivot(const TArray<AActor*>& InActors);
	static EComponentMobility::Type FindMobility(const TArray<AActor*>& InActors);
	
};


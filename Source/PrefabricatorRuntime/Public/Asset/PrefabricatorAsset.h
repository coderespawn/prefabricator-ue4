//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PrefabricatorAsset.generated.h"

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorPropertyAssetMapping {
	GENERATED_BODY()

	UPROPERTY()
	FSoftObjectPath AssetReference;

	UPROPERTY()
	FString AssetClassName;

	UPROPERTY()
	FName AssetObjectPath;

	UPROPERTY()
	bool bUseQuotes = false;
};

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorProperty : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY()
	FString PropertyName;

	UPROPERTY()
	FString ExportedValue;

	UPROPERTY()
	TArray<FPrefabricatorPropertyAssetMapping> AssetSoftReferenceMappings;

	UPROPERTY()
	bool bIsCrossReferencedActor = false;

	UPROPERTY()
	FGuid CrossReferencePrefabActorId;

	void SaveReferencedAssetValues();
	void LoadReferencedAssetValues();
};

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorComponentData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY()
	FTransform RelativeTransform;

	UPROPERTY()
	FString ComponentName;

	UPROPERTY()
	TArray<UPrefabricatorProperty*> Properties;
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
	FSoftClassPath ClassPathRef;

	UPROPERTY()
	TArray<UPrefabricatorProperty*> Properties;

	UPROPERTY()
	TArray<FPrefabricatorComponentData> Components;

#if WITH_EDITORONLY_DATA
	UPROPERTY()
	FString ActorName;
#endif // WITH_EDITORONLY_DATA
};

struct FPrefabAssetSelectionConfig {
	int32 Seed = 0;
};

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorEventListener : public UObject {
	GENERATED_BODY()
public:
	/** Called when the prefab and all its child prefabs have been spawned and initialized */
	UFUNCTION(BlueprintNativeEvent, Category = "Prefabricator")
	void PostSpawn(APrefabActor* Prefab);
	virtual void PostSpawn_Implementation(APrefabActor* Prefab);
};

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAssetInterface : public UObject {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	TSubclassOf<UPrefabricatorEventListener> EventListener;

	UPROPERTY(EditAnywhere, Category = "Replication")
	bool bReplicates = false;

public:
	virtual class UPrefabricatorAsset* GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig) { return nullptr; }
};

enum class EPrefabricatorAssetVersion {
	InitialVersion = 0,
	AddedSoftReference,
	AddedSoftReference_PrefabFix,

	//----------- Versions should be placed above this line -----------------
	LastVersionPlusOne,
	LatestVersion = LastVersionPlusOne -1
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
	UPROPERTY()
	class UThumbnailInfo* ThumbnailInfo;

	UPROPERTY()
	uint32 Version;

public:
	virtual UPrefabricatorAsset* GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig) override;
};


USTRUCT(BlueprintType)
struct PREFABRICATORRUNTIME_API FPrefabricatorAssetCollectionItem {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	TSoftObjectPtr<UPrefabricatorAsset> PrefabAsset;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	float Weight = 1.0f;
};

enum class EPrefabricatorCollectionAssetVersion {
	InitialVersion = 0,

	//----------- Versions should be placed above this line -----------------
	LastVersionPlusOne,
	LatestVersion = LastVersionPlusOne - 1
};

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAssetCollection : public UPrefabricatorAssetInterface {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	TArray<FPrefabricatorAssetCollectionItem> Prefabs;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	TSoftObjectPtr<UTexture2D> CustomThumbnail;
#endif // WITH_EDITORONLY_DATA

	UPROPERTY()
	uint32 Version;

public:
	virtual UPrefabricatorAsset* GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig) override;
};


class PREFABRICATORRUNTIME_API FPrefabricatorAssetUtils {
public:
	static FVector FindPivot(const TArray<AActor*>& InActors);
	static EComponentMobility::Type FindMobility(const TArray<AActor*>& InActors);
	
};


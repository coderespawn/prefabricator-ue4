//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"
#include "PrefabricatorAsset.generated.h"

USTRUCT()
struct PREFABRICATORRUNTIME_API FPrefabricatorActorData {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	FTransform RelativeTransform;

	UPROPERTY(VisibleAnywhere)
	UClass* ActorClass;

	UPROPERTY()
	TArray<uint8> ActorData;
};


UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabricatorAsset : public UObject {
	GENERATED_UCLASS_BODY()
public:


};



class PREFABRICATORRUNTIME_API FPrefabricatorAssetUtils {
public:
	static FVector FindPivot(const TArray<AActor*>& InActors);
	static EComponentMobility::Type FindMobility(const TArray<AActor*>& InActors);
	static void SaveActorData(AActor* InActor, const FVector& InPrefabPivot, FPrefabricatorActorData& OutActorData);
	
};
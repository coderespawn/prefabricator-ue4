//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/EngineTypes.h"

class APrefabActor;
class UPrefabricatorConstructionSnapComponent;
class UPrefabricatorAssetInterface;

class CONSTRUCTIONSYSTEMRUNTIME_API FConstructionSystemUtils {
public:
	static ECollisionChannel FindPrefabSnapChannel();
	static APrefabActor* FindTopMostPrefabActor(UPrefabricatorConstructionSnapComponent* SnapComponent);
	static APrefabActor* ConstructPrefabItem(UWorld* InWorld, UPrefabricatorAssetInterface* InPrefabAsset, const FTransform& InTransform, int32 InSeed);
	static bool GetSnapPoint(UPrefabricatorConstructionSnapComponent* InFixedSnapComp, UPrefabricatorConstructionSnapComponent* InNewSnapComp,
		const FVector& InRequestedSnapLocation, FTransform& OutTargetSnapTransform, int32 CursorRotationStep = 0, float InSnapTolerrance = 200.0f);
};

class CONSTRUCTIONSYSTEMRUNTIME_API FConstructionSystemCollision {
public:
	static bool WallWallCollision(const FVector& ExtentA, const FTransform& TransformA, const FVector& ExtentB, const FTransform& TransformB);
	static bool WallBoxCollision(const FVector& WallExtent, const FTransform& WallTransform, const FVector& BoxExtent, const FTransform& BoxTransform);
};


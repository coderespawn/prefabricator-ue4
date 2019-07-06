//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystemCursor.generated.h"

class APrefabActor;
class UPrefabricatorAssetInterface;

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemCursor : public UObject {
	GENERATED_BODY()

public:
	void RecreateCursor(UWorld* InWorld, UPrefabricatorAssetInterface* InActivePrefabAsset, UMaterialInterface* InCursorMaterial);
	void DestroyCursor();
	void SetVisiblity(bool bVisible);
	APrefabActor* GetCursorGhostActor() const { return CursorGhostActor; }
	void SetTransform(const FVector& Location, const FVector& Normal);
	bool GetCursorTransform(FTransform& OutTransform) const;

	FORCEINLINE void IncrementSeed() { ++CursorSeed;  }
	FORCEINLINE void DecrementSeed() { --CursorSeed; }
	FORCEINLINE int32 GetCursorSeed() const { return CursorSeed; }

private:
	UPROPERTY(Transient)
	APrefabActor* CursorGhostActor = nullptr;

	UPROPERTY(Transient)
	int32 CursorSeed = 0;


};

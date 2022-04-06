//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystemCursor.generated.h"

class APrefabActor;
class UPrefabricatorAssetInterface;
class UMaterialInterface;
class UPrefabricatorConstructionSnapComponent;

UENUM()
enum class EConstructionSystemCursorVisiblity : uint8 {
	Visible,
	VisibleInvalid,
	Hidden
};

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemCursor : public UObject {
	GENERATED_BODY()

public:
	void RecreateCursor(UWorld* InWorld, UPrefabricatorAssetInterface* InCursorPrefab);
	void DestroyCursor();
	void SetVisiblity(EConstructionSystemCursorVisiblity InVisiblity, bool bForce = false);
	EConstructionSystemCursorVisiblity GetVisiblity() const { return Visiblity;	}

	APrefabActor* GetCursorGhostActor() const { return CursorGhostActor; }
	void SetTransform(const FTransform& InTransform);
	bool GetCursorTransform(FTransform& OutTransform) const;

	FORCEINLINE void IncrementSeed() { ++CursorSeed;  }
	FORCEINLINE void DecrementSeed() { --CursorSeed; }
	FORCEINLINE int32 GetCursorSeed() const { return CursorSeed; }

	FORCEINLINE void SetCursorMaterial(UMaterialInterface* InCursorMaterial) { CursorMaterial = InCursorMaterial; }
	FORCEINLINE void SetCursorInvalidMaterial(UMaterialInterface* InCursorInvalidMaterial) { CursorInvalidMaterial = InCursorInvalidMaterial; }

	void MoveToNextSnapComponent();
	void MoveToPrevSnapComponent();

	UPrefabricatorConstructionSnapComponent* GetActiveSnapComponent();

private:
	void AssignMaterialRecursive(UMaterialInterface* Material) const;

private:
	UPROPERTY(Transient)
	APrefabActor* CursorGhostActor = nullptr;

	UPROPERTY(Transient)
	int32 CursorSeed = 0;

	UPROPERTY(Transient)
	UMaterialInterface* CursorMaterial;

	UPROPERTY(Transient)
	UMaterialInterface* CursorInvalidMaterial;

	UPROPERTY(Transient)
	TArray<UPrefabricatorConstructionSnapComponent*> SnapComponents;

	UPROPERTY(Transient)
	int32 ActiveSnapComponentIndex = 0;

	EConstructionSystemCursorVisiblity Visiblity = EConstructionSystemCursorVisiblity::Visible;
};


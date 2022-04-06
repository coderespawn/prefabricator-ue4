//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "ConstructionSystemSnap.generated.h"

class USphereComponent;
class UArrowComponent;

UENUM(BlueprintType)
enum class EPrefabricatorConstructionSnapType : uint8
{
	Floor,
	Wall,
	Object
};


USTRUCT(BlueprintType)
struct CONSTRUCTIONSYSTEMRUNTIME_API FPCSnapConstraintFloor {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachX = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachXNegative = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachY = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachYNegative = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachZ = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachZNegative = true;
};

USTRUCT(BlueprintType)
struct CONSTRUCTIONSYSTEMRUNTIME_API FPCSnapConstraintWall {
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachTop = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachBottom = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachLeft = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool AttachRight = true;
};


UCLASS(meta = (BlueprintSpawnableComponent))
class CONSTRUCTIONSYSTEMRUNTIME_API UPrefabricatorConstructionSnapComponent : public UBoxComponent {
	GENERATED_UCLASS_BODY()
public:
	virtual void OnRegister() override;

	//~ Begin UPrimitiveComponent Interface.
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;
	//~ End UPrimitiveComponent Interface.

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	EPrefabricatorConstructionSnapType SnapType;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	FPCSnapConstraintFloor FloorConstraint;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	FPCSnapConstraintWall WallConstraint;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bAlignToGroundSlope = false;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bUseMaxGroundSlopeConstraint = false;

	UPROPERTY(EditAnywhere, Category = "Prefabricator", Meta=(EditCondition="bUseMaxGroundSlopeConstraint"))
	float MaxGroundPlacementSlope = 60.0f;
};

UCLASS(ConversionRoot, ComponentWrapperClass)
class CONSTRUCTIONSYSTEMRUNTIME_API APrefabricatorConstructionSnap : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY(Category = "Prefabricator", VisibleAnywhere, BlueprintReadOnly, meta = (ExposeFunctionCategories = "Prefabricator", AllowPrivateAccess = "true"))
	class UPrefabricatorConstructionSnapComponent* ConstructionSnapComponent;

public:
	/** Returns StaticMeshComponent subobject **/
	class UPrefabricatorConstructionSnapComponent* GetSnapComponent() const { return ConstructionSnapComponent; }
};


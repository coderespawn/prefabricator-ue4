//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
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


UCLASS(meta = (BlueprintSpawnableComponent))
class CONSTRUCTIONSYSTEMRUNTIME_API UPrefabricatorConstructionSnapComponent : public UBoxComponent {
	GENERATED_UCLASS_BODY()
public:
	virtual void OnRegister() override;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	EPrefabricatorConstructionSnapType SnapType;
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

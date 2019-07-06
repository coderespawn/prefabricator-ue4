//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ConstructionSystemComponent.generated.h"

class UPrefabricatorAssetInterface;
class APrefabActor;
class UConstructionSystemCursor;
class UConstructionSystemTool;
class UMaterialInterface;

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UConstructionSystemComponent();

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void ToggleConstructionSystem();

	//~ Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void DestroyComponent(bool bPromoteChildren = false) override;
	virtual void BeginPlay() override;
	//~ End UActorComponent Interface

private:
	APlayerController* GetPlayerController();
	void TransitionCameraTo(AActor* InViewTarget, float InBlendTime, float InBlendExp);
	void HandleUpdate();
	void BindInput();

	void EnableConstructionSystem();
	void DisableConstructionSystem();
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	AActor* ConstructionCameraActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ConstructionCameraTransitionTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ConstructionCameraTransitionExp = 1.0f;

	UPROPERTY(Transient, BlueprintReadOnly, Category = "ConstructionSystem")
	UConstructionSystemTool* ActiveTool;

private:
	bool bConstructionSystemEnabled = false;
};
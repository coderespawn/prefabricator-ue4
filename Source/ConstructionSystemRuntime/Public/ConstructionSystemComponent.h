//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/AssetUserData.h"
#include "ConstructionSystemComponent.generated.h"

class UPrefabricatorAssetInterface;
class APrefabActor;
class UConstructionSystemCursor;
class UConstructionSystemTool;
class UMaterialInterface;
class UConstructionSystemUIAsset;
class UInputComponent;
class UUserWidget;

UENUM(BlueprintType)
enum class EConstructionSystemToolType : uint8 {
	BuildTool		UMETA(DisplayName = "Build Tool"),
	RemoveTool		UMETA(DisplayName = "Remove Tool")
};

UCLASS(BlueprintType, meta = (BlueprintSpawnableComponent))
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemComponent : public UActorComponent {
	GENERATED_BODY()
public:
	UConstructionSystemComponent();

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void ToggleConstructionSystem();

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void EnableConstructionSystem(EConstructionSystemToolType InToolType);

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void DisableConstructionSystem();

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void ShowBuildMenu();

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void HideBuildMenu();

	//~ Begin UActorComponent Interface
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void OnRegister() override;
	//~ End UActorComponent Interface

	DECLARE_DELEGATE_OneParam(FSetToolDelegate, EConstructionSystemToolType);

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	void SetActiveTool(EConstructionSystemToolType InToolType);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ConstructionSystem")
	EConstructionSystemToolType GetActiveToolType() const { return ActiveToolType; }

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ConstructionSystem")
	UConstructionSystemTool* GetActiveTool();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "ConstructionSystem")
	UConstructionSystemTool* GetTool(EConstructionSystemToolType InToolType);


private:
	APlayerController* GetPlayerController();
	APawn* GetControlledPawn();
	void TransitionCameraTo(AActor* InViewTarget, float InBlendTime, float InBlendExp);
	void HandleUpdate();
	void BindInput(UInputComponent* InputComponent);

	void ToggleBuildUI();
	void CreateTools();
	void DestroyTools();
	void _CreateTool(EConstructionSystemToolType ToolType, TSubclassOf<UConstructionSystemTool> InToolClass);

	void CreateBuildMenu();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	UMaterialInterface* CursorInvalidMaterial;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	float TraceStartDistance = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Cursor")
	float TraceSweepRadius = 40;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	AActor* ConstructionCameraActor;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ConstructionCameraTransitionTime = 0.15f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Camera")
	float ConstructionCameraTransitionExp = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<UUserWidget> BuildMenuUI;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	UConstructionSystemUIAsset* BuildMenuData;

	UPROPERTY(Transient)
	UUserWidget* BuildMenuUIInstance;

	UPROPERTY(Transient)
	EConstructionSystemToolType ActiveToolType = EConstructionSystemToolType::BuildTool;

	UPROPERTY(Transient)
	TMap<EConstructionSystemToolType, UConstructionSystemTool*> Tools;

private:
	bool bConstructionSystemEnabled = false;
	bool bInputBound = false;
};


UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemItemUserData : public UAssetUserData {
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "Prefabricator")
	int32 Seed;
};


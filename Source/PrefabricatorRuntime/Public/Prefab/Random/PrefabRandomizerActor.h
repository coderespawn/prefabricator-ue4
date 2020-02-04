//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "PrefabRandomizerActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FPrefabRandomizerCompleteBindableEvent);

typedef TSharedPtr<struct FPrefabLoadState> FPrefabLoadStatePtr;

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API APrefabRandomizer : public AActor {
	GENERATED_UCLASS_BODY()

public:
	virtual void Tick(float DeltaSeconds) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	void Randomize(int32 InSeed);

#if WITH_EDITOR
	virtual FName GetCustomIconName() const override;
#endif // WITH_EDITOR

public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bRandomizeOnBeginPlay = true;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	int32 SeedOffset = 0;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	float MaxBuildTimePerFrame = 0.02f;

	UPROPERTY(BlueprintAssignable, Category = "Prefabricator")
	FPrefabRandomizerCompleteBindableEvent OnRandomizationComplete;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bFastSyncBuild = false;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bUseOptimizedPooling = true;

private:
	TSharedPtr<class FPrefabBuildSystem> BuildSystem;
	FRandomStream Random;
	FPrefabLoadStatePtr LoadState;
};


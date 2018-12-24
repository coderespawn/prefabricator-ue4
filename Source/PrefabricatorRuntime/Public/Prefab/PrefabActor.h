//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "GameFramework/Actor.h"
#include "PrefabActor.generated.h"

UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class PREFABRICATORRUNTIME_API APrefabActor : public AActor {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(BlueprintReadOnly, meta = (ExposeFunctionCategories = "Prefabricator", AllowPrivateAccess = "true"))
	class UPrefabComponent* PrefabComponent;

public:
	/// AActor Interface 
	virtual void Destroyed() override;
	virtual void PostLoad() override;
	virtual void PostActorCreated() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;
	virtual void PostDuplicate(EDuplicateMode::Type DuplicateMode) override;
	virtual FName GetCustomIconName() const override;
#endif // WITH_EDITOR
	/// End of AActor Interface 

	UFUNCTION(BlueprintCallable)
	void LoadPrefab();

	UFUNCTION(BlueprintCallable)
	void SavePrefab();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	bool IsPrefabOutdated();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPrefabricatorAsset* GetPrefabAsset();

	void RandomizeSeed(const FRandomStream& InRandom, bool bRecursive = true);

public:
	// The last update ID of the prefab asset when this actor was refreshed from it
	// This is used to test if the prefab has changed since we last recreated it
	UPROPERTY()
	FGuid LastUpdateID;

	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	int32 Seed;
};

/////////////////////////////// BuildQueue /////////////////////////////// 

class FPrefabBuildQueue;

class PREFABRICATORRUNTIME_API FPrefabBuildQueueCommand {
public:
	virtual ~FPrefabBuildQueueCommand() {}
	virtual void Execute(FPrefabBuildQueue& Queue) = 0;
};
typedef TSharedPtr<FPrefabBuildQueueCommand> FPrefabBuildQueueCommandPtr;


class PREFABRICATORRUNTIME_API FPrefabBuildQueueCommand_BuildPrefab : public FPrefabBuildQueueCommand {
public:
	FPrefabBuildQueueCommand_BuildPrefab(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom);

	virtual void Execute(FPrefabBuildQueue& Queue) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
	bool bRandomizeNestedSeed = false;
	FRandomStream* Random = nullptr;
};

class PREFABRICATORRUNTIME_API FPrefabBuildQueueCommand_NotifyBuildComplete : public FPrefabBuildQueueCommand {
public:
	FPrefabBuildQueueCommand_NotifyBuildComplete(TWeakObjectPtr<APrefabActor> InPrefab);
	virtual void Execute(FPrefabBuildQueue& Queue) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
};


class PREFABRICATORRUNTIME_API FPrefabBuildQueue {
public:
	FPrefabBuildQueue(double InTimePerFrame);
	void Tick();
	void Reset();
	void Enqueue(FPrefabBuildQueueCommandPtr InCommand);

private:
	TArray<FPrefabBuildQueueCommandPtr> BuildQueue;
	double TimePerFrame = 0;
};
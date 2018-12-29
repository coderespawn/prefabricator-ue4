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

	UFUNCTION(BlueprintCallable)
	void RandomizeSeed(const FRandomStream& InRandom, bool bRecursive = true);
	void HandleBuildComplete();

public:
	// The last update ID of the prefab asset when this actor was refreshed from it
	// This is used to test if the prefab has changed since we last recreated it
	UPROPERTY()
	FGuid LastUpdateID;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Prefabricator")
	int32 Seed;
};

/////////////////////////////// BuildSystem /////////////////////////////// 

class FPrefabBuildSystem;

class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand {
public:
	virtual ~FPrefabBuildSystemCommand() {}
	virtual void Execute(FPrefabBuildSystem& BuildSystem) = 0;
};
typedef TSharedPtr<FPrefabBuildSystemCommand> FPrefabBuildSystemCommandPtr;


class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand_BuildPrefab : public FPrefabBuildSystemCommand {
public:
	FPrefabBuildSystemCommand_BuildPrefab(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom);

	virtual void Execute(FPrefabBuildSystem& BuildSystem) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
	bool bRandomizeNestedSeed = false;
	FRandomStream* Random = nullptr;
};

class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand_NotifyBuildComplete : public FPrefabBuildSystemCommand {
public:
	FPrefabBuildSystemCommand_NotifyBuildComplete(TWeakObjectPtr<APrefabActor> InPrefab);
	virtual void Execute(FPrefabBuildSystem& BuildSystem) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
};


class PREFABRICATORRUNTIME_API FPrefabBuildSystem {
public:
	FPrefabBuildSystem(double InTimePerFrame);
	void Tick();
	void Reset();
	void PushCommand(FPrefabBuildSystemCommandPtr InCommand);

private:
	TArray<FPrefabBuildSystemCommandPtr> BuildStack;
	double TimePerFrame = 0;
};
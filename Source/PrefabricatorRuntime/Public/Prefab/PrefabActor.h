//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "GameFramework/Actor.h"
#include "PrefabActor.generated.h"

UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class PREFABRICATORRUNTIME_API APrefabActor : public AActor {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (ExposeFunctionCategories = "Prefabricator,Mobility", AllowPrivateAccess = "true"))
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

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	void LoadPrefab();

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
	void SavePrefab();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Prefabricator")
	bool IsPrefabOutdated();

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Prefabricator")
	UPrefabricatorAsset* GetPrefabAsset();

	UFUNCTION(BlueprintCallable, Category = "Prefabricator")
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
typedef TSharedPtr<struct FPrefabLoadState> FPrefabLoadStatePtr;

class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand {
public:
	virtual ~FPrefabBuildSystemCommand() {}
	virtual void Execute(FPrefabBuildSystem& BuildSystem) = 0;
};
typedef TSharedPtr<FPrefabBuildSystemCommand> FPrefabBuildSystemCommandPtr;


class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand_BuildPrefab : public FPrefabBuildSystemCommand {
public:
	FPrefabBuildSystemCommand_BuildPrefab(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom, FPrefabLoadStatePtr InLoadState);

	virtual void Execute(FPrefabBuildSystem& BuildSystem) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
	bool bRandomizeNestedSeed = false;
	FRandomStream* Random = nullptr;
	FPrefabLoadStatePtr LoadState;
};

class PREFABRICATORRUNTIME_API FPrefabBuildSystemCommand_BuildPrefabSync : public FPrefabBuildSystemCommand {
public:
	FPrefabBuildSystemCommand_BuildPrefabSync(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom, FPrefabLoadStatePtr InLoadState);

	virtual void Execute(FPrefabBuildSystem& BuildSystem) override;

private:
	TWeakObjectPtr<APrefabActor> Prefab;
	bool bRandomizeNestedSeed = false;
	FRandomStream* Random = nullptr;
	FPrefabLoadStatePtr LoadState;
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
	int32 GetNumPendingCommands() const { return BuildStack.Num(); }

private:
	TArray<FPrefabBuildSystemCommandPtr> BuildStack;
	double TimePerFrame = 0;
};



UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class PREFABRICATORRUNTIME_API AReplicablePrefabActor : public APrefabActor {
	GENERATED_UCLASS_BODY()
public:
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void BeginPlay() override;

};


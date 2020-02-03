//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Info.h"

class IPrefabricatorService;
class UPrefabricatorAsset;
class APrefabActor;

/** This service allows the library to access editor code from the runtime module if we are running on Unreal Editor or
    fallback to the runtime service if we are on the standalone build
	Also See:
		FPrefabricatorRuntimeService
		FPrefabricatorEditorService
*/
class PREFABRICATORRUNTIME_API FPrefabricatorService {
public:
	static TSharedPtr<IPrefabricatorService> Get();
	static void Set(TSharedPtr<IPrefabricatorService> InInstance);

private:
	static TSharedPtr<IPrefabricatorService> Instance;
};


class PREFABRICATORRUNTIME_API IPrefabricatorService {
public:
	virtual ~IPrefabricatorService() {}

	virtual void ParentActors(AActor* ParentActor, AActor* ChildActor) = 0;
	virtual void SelectPrefabActor(AActor* PrefabActor) = 0;
	virtual void GetSelectedActors(TArray<AActor*>& OutActors) = 0;
	virtual int GetNumSelectedActors() = 0;
	virtual UPrefabricatorAsset* CreatePrefabAsset() = 0;
	virtual FVector SnapToGrid(const FVector& InLocation) { return InLocation; }
	virtual void SetDetailsViewObject(UObject* InObject) {}
	virtual AActor* SpawnActor(TSubclassOf<AActor> InClass, const FTransform& InTransform, ULevel* InLevel);
	virtual void BeginTransaction(const FText& Description) {}
	virtual void EndTransaction() {}
};

class PREFABRICATORRUNTIME_API FPrefabricatorRuntimeService : public IPrefabricatorService {
public:
	virtual void ParentActors(AActor* ParentActor, AActor* ChildActor) override;
	virtual void SelectPrefabActor(AActor* PrefabActor) override;
	virtual void GetSelectedActors(TArray<AActor*>& OutActors) override;
	virtual int GetNumSelectedActors() override;
	virtual UPrefabricatorAsset* CreatePrefabAsset() override;
};


//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ActorFactories/ActorFactory.h"
#include "PrefabricatorActorFactory.generated.h"

UCLASS()
class PREFABRICATOREDITOR_API UPrefabricatorActorFactory : public UActorFactory {
	GENERATED_UCLASS_BODY()

	// UActorFactory interface
	virtual UObject* GetAssetFromActorInstance(AActor* ActorInstance) override;
	virtual AActor* SpawnActor(UObject* InAsset, ULevel* InLevel, const FTransform& InTransform, const FActorSpawnParameters& InSpawnParams) override;
	virtual void PostSpawnActor(UObject* Asset, AActor* NewActor) override;
	virtual void PostCreateBlueprint(UObject* Asset, AActor* CDO) override;
	virtual bool CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg) override;
	// End of UActorFactory interface

	void LoadPrefabActorState(class APrefabActor* PrefabActor);
};


//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabComponent.h"

#include "ComponentAssetBroker.h"

/** Asset broker for the prefab asset */
class PREFABRICATOREDITOR_API FPrefabricatorAssetBroker : public IComponentAssetBroker {
public:
	virtual UClass* GetSupportedAssetClass() override {
		return UPrefabricatorAssetInterface::StaticClass();
	}

	virtual bool AssignAssetToComponent(UActorComponent* InComponent, UObject* InAsset) override {
		if (UPrefabComponent* PrefabComponent = Cast<UPrefabComponent>(InComponent)) {
			UPrefabricatorAssetInterface* PrefabAsset = Cast<UPrefabricatorAssetInterface>(InAsset);
			if (PrefabAsset && PrefabAsset) {
				PrefabComponent->PrefabAssetInterface = PrefabAsset;
				return true;
			}
		}

		return false;
	}

	virtual UObject* GetAssetFromComponent(UActorComponent* InComponent) override {
		if (UPrefabComponent* PrefabComponent = Cast<UPrefabComponent>(InComponent)) {
			return PrefabComponent->PrefabAssetInterface.LoadSynchronous();
		}
		return nullptr;
	}
};


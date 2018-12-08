//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorAssetFactory.h"
#include "PrefabricatorAsset.h"

UPrefabricatorAssetFactory::UPrefabricatorAssetFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SupportedClass = UPrefabricatorAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UPrefabricatorAssetFactory::CanCreateNew() const {
	return true;
}

UObject* UPrefabricatorAssetFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) {
	UPrefabricatorAsset* NewAsset = NewObject<UPrefabricatorAsset>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}


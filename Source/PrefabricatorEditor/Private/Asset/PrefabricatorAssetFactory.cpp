//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAssetFactory.h"

#include "Asset/PrefabricatorAsset.h"
#include "Utils/PrefabEditorTools.h"

////////////////////////////////// UPrefabricatorAssetFactory ///////////////////////////////////

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
	NewAsset->Version = (uint32)EPrefabricatorAssetVersion::LatestVersion;
	NewAsset->ThumbnailInfo = FPrefabEditorTools::CreateDefaultThumbInfo(NewAsset);
	return NewAsset;
}

/////////////////////////////////// UPrefabricatorAssetCollectionFactory ///////////////////////////////////

UPrefabricatorAssetCollectionFactory::UPrefabricatorAssetCollectionFactory(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	SupportedClass = UPrefabricatorAssetCollection::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

bool UPrefabricatorAssetCollectionFactory::CanCreateNew() const
{
	return true;
}

UObject* UPrefabricatorAssetCollectionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	UPrefabricatorAssetCollection* NewAsset = NewObject<UPrefabricatorAssetCollection>(InParent, Class, Name, Flags | RF_Transactional);
	return NewAsset;
}


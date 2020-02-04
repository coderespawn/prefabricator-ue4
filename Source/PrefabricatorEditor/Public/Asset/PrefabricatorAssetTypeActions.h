//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"

class UPrefabricatorAsset;

class PREFABRICATOREDITOR_API FPrefabricatorAssetTypeActions : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return true; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override;
	virtual void OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor = TSharedPtr<IToolkitHost>()) override;
	virtual class UThumbnailInfo* GetThumbnailInfo(UObject* Asset) const override;
	virtual uint32 GetCategories() override;
	virtual FText GetDisplayNameFromAssetData(const FAssetData& AssetData) const;
	// End of IAssetTypeActions interface

	void ExecuteCreatePrefabCollection(TArray<TWeakObjectPtr<UPrefabricatorAsset>> InPrefabAssetPtrs);
	void ExecuteUpgradePrefabs(TArray<TWeakObjectPtr<UPrefabricatorAsset>> InPrefabAssetPtrs);
};


class PREFABRICATOREDITOR_API FPrefabricatorAssetCollectionTypeActions : public FAssetTypeActions_Base
{
public:
	// IAssetTypeActions interface
	virtual FText GetName() const override;
	virtual FColor GetTypeColor() const override;
	virtual UClass* GetSupportedClass() const override;
	virtual bool HasActions(const TArray<UObject*>& InObjects) const override { return false; }
	virtual void GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder) override {}
	virtual uint32 GetCategories() override;
	// End of IAssetTypeActions interface

};


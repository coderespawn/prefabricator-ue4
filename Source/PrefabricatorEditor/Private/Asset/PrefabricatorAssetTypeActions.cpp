//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAssetTypeActions.h"

#include "Asset/PrefabricatorAsset.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FDungeonThemeAssetTypeActions

FText FPrefabricatorAssetTypeActions::GetName() const
{
	return LOCTEXT("TypeActionsName", "Prefab");
}

FColor FPrefabricatorAssetTypeActions::GetTypeColor() const
{
	return FColor(99, 172, 229);
}

UClass* FPrefabricatorAssetTypeActions::GetSupportedClass() const
{
	return UPrefabricatorAsset::StaticClass();
}

void FPrefabricatorAssetTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor)
{
	const EToolkitMode::Type Mode = EditWithinLevelEditor.IsValid() ? EToolkitMode::WorldCentric : EToolkitMode::Standalone;
	for (auto ObjIt = InObjects.CreateConstIterator(); ObjIt; ++ObjIt)
	{
		UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(*ObjIt);
		if (PrefabAsset) {
			// TODO: Launch a preview editor
		}
	}
}

uint32 FPrefabricatorAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc;
}

void FPrefabricatorAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
}

#undef LOCTEXT_NAMESPACE


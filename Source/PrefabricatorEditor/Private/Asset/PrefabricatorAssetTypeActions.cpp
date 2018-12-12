//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAssetTypeActions.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabTools.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
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

void FPrefabricatorAssetTypeActions::CaptureThumbnail()
{

}

void FPrefabricatorAssetTypeActions::ExecuteCaptureThumbnails(TArray<TWeakObjectPtr<UPrefabricatorAsset>> PrefabAssets)
{
	for (TWeakObjectPtr<UPrefabricatorAsset> PrefabAsset : PrefabAssets) {
		if (PrefabAsset.IsValid()) {
			FPrefabTools::UpdatePrefabThumbnail(PrefabAsset.Get());
		}
	}
}

void FPrefabricatorAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto PrefabAssets = GetTypedWeakObjectPtrs<UPrefabricatorAsset>(InObjects);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_CaptureThumb", "Capture Thumbnail"),
		NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_CaptureThumbTooltip", "Captures the prefab's thumbnail from the editor's viewport"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FPrefabricatorAssetTypeActions::ExecuteCaptureThumbnails, PrefabAssets),
			FCanExecuteAction()
		)
	);
}

#undef LOCTEXT_NAMESPACE


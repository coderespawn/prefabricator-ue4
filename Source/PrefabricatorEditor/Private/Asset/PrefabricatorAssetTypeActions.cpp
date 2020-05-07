//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAssetTypeActions.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabTools.h"
#include "PrefabricatorEditorModule.h"
#include "Utils/PrefabEditorTools.h"

#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#define LOCTEXT_NAMESPACE "AssetTypeActions"

//////////////////////////////////////////////////////////////////////////
// FPrefabricatorAssetTypeActions

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
	// Open the default editor
	FSimpleAssetEditor::CreateEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

class UThumbnailInfo* FPrefabricatorAssetTypeActions::GetThumbnailInfo(UObject* Asset) const
{
	UPrefabricatorAsset* PrefabAsset = CastChecked<UPrefabricatorAsset>(Asset);

	if (!PrefabAsset->ThumbnailInfo)
	{
		PrefabAsset->ThumbnailInfo = FPrefabEditorTools::CreateDefaultThumbInfo(PrefabAsset);
	}

	return PrefabAsset->ThumbnailInfo;
}

uint32 FPrefabricatorAssetTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc |
		IPrefabricatorEditorModule::Get().GetPrefabricatorAssetCategoryBit();
}

FText FPrefabricatorAssetTypeActions::GetDisplayNameFromAssetData(const FAssetData& AssetData) const
{
	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(AssetData.FastGetAsset(true));
	if (PrefabAsset) {
		if (PrefabAsset->Version != (uint32)EPrefabricatorAssetVersion::LatestVersion) {
			return LOCTEXT("TypeActionsName_UpgradeRequired", "Prefab (upgrade required, right click)");
		}
	}
	return FText::GetEmpty();
}

void FPrefabricatorAssetTypeActions::ExecuteCreatePrefabCollection(TArray<TWeakObjectPtr<UPrefabricatorAsset>> InPrefabAssetPtrs)
{
	TArray<UPrefabricatorAsset*> PrefabAssets;
	{
		for (TWeakObjectPtr<UPrefabricatorAsset> AssetPtr : InPrefabAssetPtrs) {
			if (AssetPtr.IsValid()) {
				PrefabAssets.Add(AssetPtr.Get());
			}
		}
	}

	UPrefabricatorAssetCollection* Collection = FPrefabEditorTools::CreatePrefabCollectionAsset();
	for (UPrefabricatorAsset* PrefabAsset : PrefabAssets) {
		FPrefabricatorAssetCollectionItem Item;
		Item.PrefabAsset = PrefabAsset;
		Collection->Prefabs.Add(Item);
	}
	Collection->Modify();
}

void FPrefabricatorAssetTypeActions::ExecuteUpgradePrefabs(TArray<TWeakObjectPtr<UPrefabricatorAsset>> InPrefabAssetPtrs)
{
for (TWeakObjectPtr<UPrefabricatorAsset> PrefabAssetPtr : InPrefabAssetPtrs) {
	if (PrefabAssetPtr.IsValid()) {
		UPrefabricatorAsset* PrefabAsset = PrefabAssetPtr.Get();
		if (PrefabAsset->Version != (uint32)EPrefabricatorAssetVersion::LatestVersion) {
			FPrefabVersionControl::UpgradeToLatestVersion(PrefabAsset);
		}
	}
}
}

void FPrefabricatorAssetTypeActions::ExecuteRecaptureThumbnails(TArray<TWeakObjectPtr<UPrefabricatorAsset>> InPrefabAssetPtrs)
{
	for (TWeakObjectPtr<UPrefabricatorAsset> PrefabAssetPtr : InPrefabAssetPtrs) {
		if (PrefabAssetPtr.IsValid()) {
			UPrefabricatorAsset* PrefabAsset = PrefabAssetPtr.Get();
			FPrefabEditorTools::CapturePrefabAssetThumbnail(PrefabAsset);
		}
	}

}

void FPrefabricatorAssetTypeActions::GetActions(const TArray<UObject*>& InObjects, FMenuBuilder& MenuBuilder)
{
	auto PrefabAssets = GetTypedWeakObjectPtrs<UPrefabricatorAsset>(InObjects);

	bool bUpgradeRequired = false;
	for (TWeakObjectPtr<UPrefabricatorAsset> PrefabAssetPtr : PrefabAssets) {
		if (PrefabAssetPtr.IsValid()) {
			if (PrefabAssetPtr->Version != (uint32)EPrefabricatorAssetVersion::LatestVersion) {
				bUpgradeRequired = true;
				break;
			}
		}
	}

	if (bUpgradeRequired) {
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_UpgradeAsset", "Upgrade to latest version"),
			NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_UpgradeAssetTooltip", "Upgrades the prefab asset to the latest version"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FPrefabricatorAssetTypeActions::ExecuteUpgradePrefabs, PrefabAssets),
				FCanExecuteAction()
			)
		);
	}

	if (PrefabAssets.Num() > 0) {
		MenuBuilder.AddMenuEntry(
			NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_CreatePrefabCollection", "Create Prefab Collection"),
			NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_CreatePrefabCollectionTooltip", "Creates a prefab collection, which can be used to randomly select one based on weights"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &FPrefabricatorAssetTypeActions::ExecuteCreatePrefabCollection, PrefabAssets),
				FCanExecuteAction()
			)
		);
	}


	// Recapture thumbnail
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_RecaptureThumb", "Recapture Thumbnail"),
		NSLOCTEXT("AssetTypeActions_PrefabricatorAsset", "ObjectContext_RecaptureThumbTooltip", "Recaptures the thumbnail of the selected prefab assets"),
		FSlateIcon(),
		FUIAction(
			FExecuteAction::CreateSP(this, &FPrefabricatorAssetTypeActions::ExecuteRecaptureThumbnails, PrefabAssets),
			FCanExecuteAction()
		)
	);
}

//////////////////////////////////////////////////////////////////////////
// FPrefabricatorAssetCollectionTypeActions

FText FPrefabricatorAssetCollectionTypeActions::GetName() const
{
	return LOCTEXT("TypeActionCollectionName", "Prefab Collection");
}

FColor FPrefabricatorAssetCollectionTypeActions::GetTypeColor() const
{
	return FColor(99, 229, 172);
}

UClass* FPrefabricatorAssetCollectionTypeActions::GetSupportedClass() const
{
	return UPrefabricatorAssetCollection::StaticClass();
}

class FPrefabCollectionAssetEditor : public FSimpleAssetEditor {
public:
	void InitCollectionEditor(const EToolkitMode::Type Mode, const TSharedPtr< class IToolkitHost >& InitToolkitHost, const TArray<UObject*>& ObjectsToEdit) {
		AssetObjects = ObjectsToEdit;
		InitEditor(Mode, InitToolkitHost, ObjectsToEdit, FSimpleAssetEditor::FGetDetailsViewObjects());
	}

	virtual void SaveAsset_Execute() override {
		for (UObject* Obj : AssetObjects) {
			if (UPrefabricatorAssetCollection* Collection = Cast<UPrefabricatorAssetCollection>(Obj)) {
				UTexture2D* CustomThumb = Collection->CustomThumbnail.LoadSynchronous();
				if (CustomThumb) {
					FPrefabEditorTools::AssignPrefabAssetThumbnail(Collection, CustomThumb);
				}
			}
		}
		FSimpleAssetEditor::SaveAsset_Execute();
	}

private:
	TArray<UObject*> AssetObjects;
};


void FPrefabricatorAssetCollectionTypeActions::OpenAssetEditor(const TArray<UObject*>& InObjects, TSharedPtr<class IToolkitHost> EditWithinLevelEditor /*= TSharedPtr<IToolkitHost>()*/)
{
	TSharedRef<FPrefabCollectionAssetEditor> NewEditor(new FPrefabCollectionAssetEditor());
	NewEditor->InitCollectionEditor(EToolkitMode::Standalone, EditWithinLevelEditor, InObjects);
}

uint32 FPrefabricatorAssetCollectionTypeActions::GetCategories()
{
	return EAssetTypeCategories::Misc |
		IPrefabricatorEditorModule::Get().GetPrefabricatorAssetCategoryBit();
}

#undef LOCTEXT_NAMESPACE


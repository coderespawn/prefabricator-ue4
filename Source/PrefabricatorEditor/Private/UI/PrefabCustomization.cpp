//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "UI/PrefabCustomization.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "Widgets/SBoxPanel.h"
#include "PrefabEditorTools.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "ModuleManager.h"
#include "PrefabricatorAsset.h"

#define LOCTEXT_NAMESPACE "PrefabActorCustomization" 

namespace {
	template<typename T>
	T* GetDetailObject(IDetailLayoutBuilder* DetailBuilder) {
		TArray<TWeakObjectPtr<UObject>> OutObjects;
		DetailBuilder->GetObjectsBeingCustomized(OutObjects);
		T* Obj = nullptr;
		if (OutObjects.Num() > 0) {
			Obj = Cast<T>(OutObjects[0].Get());
		}
		return Obj;
	}
}

void FPrefabActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefabricator");

	Category.AddCustomRow(LOCTEXT("PrefabCommand_Filter", "save load prefab asset"))
	.WholeRowContent()
	[
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(4.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("PrefabCommand_SaveToAsset", "Save Prefab to Asset"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleSaveToAsset, &DetailBuilder))
		]
		
		+SHorizontalBox::Slot()
		.VAlign(VAlign_Center)
		.FillWidth(1.0f)
		.Padding(4.0f)
		[
			SNew(SButton)
			.Text(LOCTEXT("PrefabCommand_LoadFromAsset", "Load Prefab to Asset"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleLoadFromAsset, &DetailBuilder))
		]

	];
}

TSharedRef<IDetailCustomization> FPrefabActorCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabActorCustomization);
}

FReply FPrefabActorCustomization::HandleSaveToAsset(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(DetailBuilder);
	if (PrefabActor) {
		FPrefabTools::SaveStateToPrefabAsset(PrefabActor);
		FPrefabTools::UpdatePrefabThumbnail(PrefabActor->PrefabComponent->PrefabAsset);

		UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
		if (PrefabAsset) {
			// Refresh all the existing prefabs in the level
			FPrefabEditorTools::ReloadPrefabsInLevel(PrefabActor->GetWorld(), PrefabAsset);
		}

#if 0
		// Focus the content browser to this asset
		//IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
		//ContentBrowserSingleton.SyncBrowserToAssets(TArray<UObject*>({ PrefabAsset }));
#endif

	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::HandleLoadFromAsset(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(DetailBuilder);
	if (PrefabActor) {
		FPrefabTools::LoadStateFromPrefabAsset(PrefabActor);
	}
	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE 


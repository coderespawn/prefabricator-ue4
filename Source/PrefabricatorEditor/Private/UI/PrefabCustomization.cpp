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
#include "PrefabRandomizerActor.h"

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

///////////////////////////////// FPrefabActorCustomization /////////////////////////////////

void FPrefabActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(&DetailBuilder);
	UPrefabricatorAssetInterface* Asset = PrefabActor ? PrefabActor->PrefabComponent->PrefabAssetInterface : nullptr;

	if (Asset) {
		if (Asset->IsA<UPrefabricatorAsset>()) {
			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Asset Actions", FText::GetEmpty(), ECategoryPriority::Important);
			Category.AddExternalObjectProperty({ PrefabActor->PrefabComponent }, GET_MEMBER_NAME_CHECKED(UPrefabComponent, PrefabAssetInterface));

			Category.AddCustomRow(LOCTEXT("PrefabCommand_Filter", "save load prefab asset"))
			.WholeRowContent()
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				//.Padding(4.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("PrefabCommand_SaveToAsset", "Save Prefab to Asset"))
					.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleSaveToAsset, &DetailBuilder))
				]
		
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				//.Padding(4.0f)
				[
					SNew(SButton)
					.Text(LOCTEXT("PrefabCommand_LoadFromAsset", "Load Prefab from Asset"))
					.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleLoadFromAsset, &DetailBuilder))
				]

			];

			Category.AddCustomRow(LOCTEXT("PrefabCommandRandomize_Filter", "randomize prefab collection asset"))
				.WholeRowContent()
				[
					SNew(SButton)
					.Text(LOCTEXT("PrefabCommand_RandomizeCollection", "Randomize"))
					.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::RandomizePrefabCollection, &DetailBuilder))
				];

			DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(APrefabActor, Seed));
		}
		else if (Asset->IsA<UPrefabricatorAssetCollection>()) {
			IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Collection Actions", FText::GetEmpty(), ECategoryPriority::Important);
			Category.AddExternalObjectProperty({ PrefabActor->PrefabComponent }, GET_MEMBER_NAME_CHECKED(UPrefabComponent, PrefabAssetInterface));

			Category.AddProperty(GET_MEMBER_NAME_CHECKED(APrefabActor, Seed));
			
			Category.AddCustomRow(LOCTEXT("PrefabCollectionCommandRandomize_Filter", "randomize prefab collection asset"))
			.WholeRowContent()
			[
				SNew(SButton)
				.Text(LOCTEXT("PrefabCollectionCommand_RandomizeCollection", "Randomize"))
				.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::RandomizePrefabCollection, &DetailBuilder))
			];

			Category.AddCustomRow(LOCTEXT("PrefabCollectionCommand_Filter", "load prefab collection asset"))
			.WholeRowContent()
			[
				SNew(SButton)
				.Text(LOCTEXT("PrefabCollectionCommand_RecreateCollection", "Reload Prefab"))
				.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleLoadFromAsset, &DetailBuilder))
			];

		}

	}
}

TSharedRef<IDetailCustomization> FPrefabActorCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabActorCustomization);
}

FReply FPrefabActorCustomization::HandleSaveToAsset(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(DetailBuilder);
	if (PrefabActor) {
		PrefabActor->SavePrefab();

		UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(PrefabActor->PrefabComponent->PrefabAssetInterface);
		if (PrefabAsset) {
			// Refresh all the existing prefabs in the level
			FPrefabEditorTools::ReloadPrefabsInLevel(PrefabActor->GetWorld(), PrefabAsset);
		}
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::HandleLoadFromAsset(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(DetailBuilder);
	if (PrefabActor) {
		PrefabActor->LoadPrefab();
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::RandomizePrefabCollection(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabActor* PrefabActor = GetDetailObject<APrefabActor>(DetailBuilder);
	if (PrefabActor) {
		FRandomStream Random;
		Random.Initialize(FMath::Rand());
		PrefabActor->RandomizeSeed(Random);
		PrefabActor->LoadPrefab();
	}
	return FReply::Handled();
}

///////////////////////////////// FPrefabRandomizerCustomization /////////////////////////////////

void FPrefabRandomizerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	APrefabRandomizer* PrefabRandomizer = GetDetailObject<APrefabRandomizer>(&DetailBuilder);

	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Randomizer", FText::GetEmpty(), ECategoryPriority::Important);
	Category.AddCustomRow(LOCTEXT("PrefabRandomizerCommand_Filter", "randomize prefab collection asset"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PrefabRandomizerCommand_Randomize", "Randomize"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabRandomizerCustomization::HandleRandomize, &DetailBuilder))
		];
}


TSharedRef<IDetailCustomization> FPrefabRandomizerCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabRandomizerCustomization);
}

FReply FPrefabRandomizerCustomization::HandleRandomize(IDetailLayoutBuilder* DetailBuilder)
{
	APrefabRandomizer* PrefabRandomizer = GetDetailObject<APrefabRandomizer>(DetailBuilder);

	FRandomStream Random;
	Random.Initialize(FMath::Rand());
	PrefabRandomizer->Randomize(Random);

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE 
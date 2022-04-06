//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "UI/PrefabCustomization.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"
#include "Prefab/Random/PrefabRandomizerActor.h"
#include "PrefabricatorEditorModule.h"
#include "PrefabricatorSettings.h"
#include "Utils/Debug/PrefabDebugActor.h"
#include "Utils/PrefabEditorTools.h"

#include "ContentBrowserModule.h"
#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "EditorFramework/ThumbnailInfo.h"
#include "EditorViewportClient.h"
#include "IContentBrowserSingleton.h"
#include "Misc/AssertionMacros.h"
#include "Modules/ModuleManager.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/SBoxPanel.h"

#define LOCTEXT_NAMESPACE "PrefabActorCustomization" 

namespace {
	template<typename T>
	TArray<T*> GetDetailObject(IDetailLayoutBuilder* DetailBuilder) {
		TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
		DetailBuilder->GetObjectsBeingCustomized(CustomizedObjects);
		TArray<T*> Result;
		for (TWeakObjectPtr<UObject> CustomizedObject : CustomizedObjects) {
			T* Obj = Cast<T>(CustomizedObject.Get());
			if (Obj) {
				Result.Add(Obj);
			}
		}
		return Result;
	}
}

///////////////////////////////// FPrefabActorCustomization /////////////////////////////////

void FPrefabActorCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(&DetailBuilder);
	TArray<UObject*> PrefabComponents;
	bool bIsCollection = false;
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor->PrefabComponent) {
			PrefabComponents.Add(PrefabActor->PrefabComponent);

			UPrefabricatorAssetInterface* Asset = PrefabActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous();
			if (Asset && Asset->IsA<UPrefabricatorAssetCollection>()) {
				bIsCollection = true;
				break;
			}
		}

	}
	FPrefabDetailsExtend& ExtenderDelegate = IPrefabricatorEditorModule::Get().GetPrefabActorDetailsExtender();

	
	if(ExtenderDelegate.IsBound())
	{
		ExtenderDelegate.Execute(DetailBuilder);
	}
	

	if (!bIsCollection) {
		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Asset Actions", FText::GetEmpty(), ECategoryPriority::Important);
		Category.AddExternalObjectProperty(PrefabComponents, GET_MEMBER_NAME_CHECKED(UPrefabComponent, PrefabAssetInterface));

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
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.FillWidth(1.0f)
			//.Padding(4.0f)
			[
				SNew(SButton)
				.Text(LOCTEXT("PrefabCommand_SaveToNewAsset", "Save Prefab to New Asset"))
				.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::HandleSaveToNewAsset, &DetailBuilder))
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


		Category.AddCustomRow(LOCTEXT("PrefabCommandUnlink_Filter", "unlink prefab"))
			.WholeRowContent()
			[
				SNew(SButton)
				.Text(LOCTEXT("PrefabCommand_Unlink", "Unlink"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabActorCustomization::UnlinkPrefab, &DetailBuilder))
			];


		DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(APrefabActor, Seed));
	}
	else {
		IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Collection Actions", FText::GetEmpty(), ECategoryPriority::Important);
		Category.AddExternalObjectProperty(PrefabComponents, GET_MEMBER_NAME_CHECKED(UPrefabComponent, PrefabAssetInterface));

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

TSharedRef<IDetailCustomization> FPrefabActorCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabActorCustomization);
}

FReply FPrefabActorCustomization::HandleSaveToAsset(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(DetailBuilder);
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor) {
			PrefabActor->SavePrefab();

			UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(PrefabActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous());
			if (PrefabAsset) {
				const UPrefabricatorSettings* PS = GetDefault<UPrefabricatorSettings>();
				if(PS->bAllowDynamicUpdate)
				{
					// Refresh all the existing prefabs in the level
					FPrefabEditorTools::ReloadPrefabsInLevel(PrefabActor->GetWorld(), PrefabAsset);
				}
			}
		}
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::HandleSaveToNewAsset(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(DetailBuilder);
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor) {
			
			TArray<AActor*> Children;
			PrefabActor->GetAttachedActors(Children);

			if(Children.Num() > 0) {
				FPrefabTools::UnlinkAndDestroyPrefabActor(PrefabActor);
				FPrefabTools::CreatePrefabFromActors(Children);
			}
		}
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::HandleLoadFromAsset(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(DetailBuilder);
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor) {
			PrefabActor->LoadPrefab();
		}
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::RandomizePrefabCollection(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(DetailBuilder);
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor) {
			FRandomStream Random;
			Random.Initialize(FMath::Rand());
			PrefabActor->RandomizeSeed(Random);

			FPrefabLoadSettings LoadSettings;
			LoadSettings.bRandomizeNestedSeed = true;
			LoadSettings.Random = &Random;
			FPrefabTools::LoadStateFromPrefabAsset(PrefabActor, LoadSettings);
		}
	}
	return FReply::Handled();
}

FReply FPrefabActorCustomization::UnlinkPrefab(IDetailLayoutBuilder* DetailBuilder)
{

	TArray<APrefabActor*> PrefabActors = GetDetailObject<APrefabActor>(DetailBuilder);
	for (APrefabActor* PrefabActor : PrefabActors) {
		if (PrefabActor) {
			FPrefabTools::UnlinkAndDestroyPrefabActor(PrefabActor);
		}
	}
	return FReply::Handled();
}

///////////////////////////////// FPrefabRandomizerCustomization /////////////////////////////////

void FPrefabRandomizerCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
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
	TArray<APrefabRandomizer*> PrefabRandomizers = GetDetailObject<APrefabRandomizer>(DetailBuilder);
	for (APrefabRandomizer* PrefabRandomizer : PrefabRandomizers) {
		if (PrefabRandomizer) {
			if (PrefabRandomizer->MaxBuildTimePerFrame > 0) {
				// This requires async build. We need to switch the editor to realtime mode
				FPrefabEditorTools::SwitchLevelViewportToRealtimeMode();
			}

			PrefabRandomizer->Randomize(FMath::Rand());
		}
	}

	return FReply::Handled();
}

///////////////////////////// FPrefabricatorAssetCustomization ///////////////////////////// 

void FPrefabricatorAssetCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<UPrefabricatorAsset*> Assets = GetDetailObject<UPrefabricatorAsset>(&DetailBuilder);
	TArray<UObject*> ThumbList;
	for (UPrefabricatorAsset* Asset : Assets) {
		if (Asset->ThumbnailInfo) {
			ThumbList.Add(Asset->ThumbnailInfo);
		}
	}
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Thumbnail", FText::GetEmpty(), ECategoryPriority::Uncommon);
	Category.AddExternalObjectProperty(ThumbList, GET_MEMBER_NAME_CHECKED(USceneThumbnailInfo, OrbitPitch));
	Category.AddExternalObjectProperty(ThumbList, GET_MEMBER_NAME_CHECKED(USceneThumbnailInfo, OrbitYaw));
	Category.AddExternalObjectProperty(ThumbList, GET_MEMBER_NAME_CHECKED(USceneThumbnailInfo, OrbitZoom));
}

TSharedRef<IDetailCustomization> FPrefabricatorAssetCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabricatorAssetCustomization);
}

///////////////////////////////// FPrefabRandomizerCustomization /////////////////////////////////

void FPrefabDebugCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	IDetailCategoryBuilder& Category = DetailBuilder.EditCategory("Prefab Debug", FText::GetEmpty(), ECategoryPriority::Important);
	Category.AddCustomRow(LOCTEXT("PrefabDebugCommand_SaveFilter", "save debug prefab"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PrefabDebugCommand_Save", "Save Actor Data"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabDebugCustomization::SaveDebugData, &DetailBuilder))
		];

	Category.AddCustomRow(LOCTEXT("PrefabDebugCommand_LoadFilter", "load debug prefab"))
		.WholeRowContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PrefabDebugCommand_lOAD", "Load Actor Data"))
			.OnClicked(FOnClicked::CreateStatic(&FPrefabDebugCustomization::LoadDebugData, &DetailBuilder))
		];

}

TSharedRef<IDetailCustomization> FPrefabDebugCustomization::MakeInstance()
{
	return MakeShareable(new FPrefabDebugCustomization);
}

FReply FPrefabDebugCustomization::SaveDebugData(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabDebugActor*> DebugActors = GetDetailObject<APrefabDebugActor>(DetailBuilder);
	for (APrefabDebugActor* DebugActor : DebugActors) {
		if (DebugActor) {
			DebugActor->SaveActorData();
		}
	}

	return FReply::Handled();
}

FReply FPrefabDebugCustomization::LoadDebugData(IDetailLayoutBuilder* DetailBuilder)
{
	TArray<APrefabDebugActor*> DebugActors = GetDetailObject<APrefabDebugActor>(DetailBuilder);
	for (APrefabDebugActor* DebugActor : DebugActors) {
		if (DebugActor) {
			DebugActor->LoadActorData();
		}
	}

	return FReply::Handled();
}

#undef LOCTEXT_NAMESPACE 


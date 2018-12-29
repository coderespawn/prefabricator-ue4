//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorEditorModule.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAssetBroker.h"
#include "Asset/PrefabricatorAssetTypeActions.h"
#include "Asset/Thumbnail/PrefabricatorAssetThumbnailRenderer.h"
#include "PrefabEditorCommands.h"
#include "PrefabEditorStyle.h"
#include "UI/EditorUIExtender.h"
#include "UI/PrefabCustomization.h"
#include "Utils/MapChangeHook.h"
#include "Utils/PrefabricatorEditorService.h"
#include "Utils/SelectionHook.h"

#include "AssetToolsModule.h"
#include "Editor/UnrealEdEngine.h"
#include "IAssetTools.h"
#include "LevelEditor.h"
#include "PropertyEditorModule.h"
#include "ThumbnailRendering/ThumbnailManager.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "PrefabricatorEditorModule" 



class FPrefabricatorEditorModule : public IPrefabricatorEditorModule
{
	virtual void StartupModule() override 
	{
		FPrefabEditorStyle::Initialize();
		FPrefabricatorCommands::Register();

		// Extend the editor menu and toolbar
		UIExtender.Extend();
		SelectionHook.Initialize();
		MapChangeHook.Initialize();

		// Register asset types
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FPrefabricatorAssetTypeActions));
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FPrefabricatorAssetCollectionTypeActions));

		// Add a category for the prefabricator assets in the context menu
		PrefabricatorAssetCategoryBit = AssetTools.RegisterAdvancedAssetCategory(FName(TEXT("Prefabricator")), LOCTEXT("PrefabricatorAssetCategory", "Prefabricator"));

		// Register the details customization
		FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
		PropertyEditorModule.RegisterCustomClassLayout("PrefabActor", FOnGetDetailCustomizationInstance::CreateStatic(&FPrefabActorCustomization::MakeInstance));
		PropertyEditorModule.RegisterCustomClassLayout("PrefabRandomizer", FOnGetDetailCustomizationInstance::CreateStatic(&FPrefabRandomizerCustomization::MakeInstance));
		
		// Register the asset brokers (used for asset to component mapping)
		PrefabAssetBroker = MakeShareable(new FPrefabricatorAssetBroker);
		FComponentAssetBrokerage::RegisterBroker(PrefabAssetBroker, UPrefabComponent::StaticClass(), true, true);

		// Override the prefabricator service with the editor version, so the runtime module can access it
		FPrefabricatorService::Set(MakeShareable(new FPrefabricatorEditorService));

		// Setup the thumbnail renderer for the prefab asset
		UThumbnailManager::Get().RegisterCustomRenderer(UPrefabricatorAsset::StaticClass(), UPrefabricatorAssetThumbnailRenderer::StaticClass());
	}

	virtual void ShutdownModule() override {

		// Unregister the prefabricator asset broker
		if (PrefabAssetBroker.IsValid()) {
			FComponentAssetBrokerage::UnregisterBroker(PrefabAssetBroker);
			PrefabAssetBroker = nullptr;
		}


		// Unregister all the asset types that we registered
		if (FModuleManager::Get().IsModuleLoaded("AssetTools"))
		{
			IAssetTools& AssetTools = FModuleManager::GetModuleChecked<FAssetToolsModule>("AssetTools").Get();
			for (int32 Index = 0; Index < CreatedAssetTypeActions.Num(); ++Index)
			{
				AssetTools.UnregisterAssetTypeActions(CreatedAssetTypeActions[Index].ToSharedRef());
			}
		}
		CreatedAssetTypeActions.Empty();

		MapChangeHook.Release();
		SelectionHook.Release();
		UIExtender.Release();

		FPrefabricatorCommands::Unregister();
		FPrefabEditorStyle::Shutdown();
	}

	virtual EAssetTypeCategories::Type GetPrefabricatorAssetCategoryBit() const override {
		return PrefabricatorAssetCategoryBit;
	}

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}
	
	FEditorUIExtender UIExtender;
	FPrefabricatorSelectionHook SelectionHook;
	FMapChangeHook MapChangeHook;
	TSharedPtr<IComponentAssetBroker> PrefabAssetBroker;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
	EAssetTypeCategories::Type PrefabricatorAssetCategoryBit;
};

IMPLEMENT_MODULE(FPrefabricatorEditorModule, PrefabricatorEditor)


#undef LOCTEXT_NAMESPACE


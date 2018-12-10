//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorEditorModule.h"

#include "Asset/PrefabricatorAssetBroker.h"
#include "Asset/PrefabricatorAssetTypeActions.h"
#include "PrefabEditorCommands.h"
#include "PrefabEditorStyle.h"
#include "Utils/PrefabEditorTools.h"

#include "AssetToolsModule.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "IAssetTools.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule" 



class FPrefabricatorEditorModule : public IPrefabricatorEditorModule
{
	virtual void StartupModule() override 
	{
		FPrefabEditorStyle::Initialize();
		FPrefabricatorCommands::Register();

		struct Local {

			static TSharedRef<FExtender> OnExtendLevelEditorActorContextMenu(const TSharedRef<FUICommandList> CommandList, const TArray<AActor*> SelectedActors) {
				TSharedRef<FExtender> Extender(new FExtender());

				if (SelectedActors.Num() > 0)
				{
					// Add asset actions extender
					Extender->AddMenuExtension(
						"ActorControl",
						EExtensionHook::After,
						CommandList,
						FMenuExtensionDelegate::CreateStatic(&Local::CreateActionMenu, SelectedActors));
				}

				return Extender;
			}

			static void CreateActionMenu(class FMenuBuilder& MenuBuilder, const TArray<AActor*> SelectedActors) {
				MenuBuilder.AddMenuEntry
				(
					LOCTEXT("PrefabTabTitle", "Create Prefab (from selection)"),
					LOCTEXT("PrefabTooltipText", "Create a new prefab from the current selection"),
					FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "Prefabricator.ContextMenu.Icon"),
					FUIAction
					(
						FExecuteAction::CreateStatic(&Local::CreatePrefabFromSelection)
					)
				);
			}

			static void ExtendLevelToolbar(FToolBarBuilder& ToolbarBuilder) {
				// Add a button to open a TimecodeSynchronizer Editor
				ToolbarBuilder.AddToolBarButton(
					FUIAction(
						FExecuteAction::CreateStatic(&FPrefabEditorTools::CreatePrefab),
						FCanExecuteAction::CreateStatic(&FPrefabEditorTools::CanCreatePrefab)
					),
					NAME_None,
					LOCTEXT("PrefabToolbarButtonText", "Create Prefab"),
					LOCTEXT("PrefabToolbarButtonTooltip", "Create a prefab from selection"),
					FSlateIcon(FPrefabEditorStyle::GetStyleSetName(), TEXT("Prefabricator.CreatePrefab"))
				);

			}

			static void CreatePrefabFromSelection()
			{
				
			}

		};

		FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
		auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

		MenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&Local::OnExtendLevelEditorActorContextMenu));
		LevelViewportExtenderHandle = MenuExtenders.Last().GetHandle();
		
		LevelToolbarExtender = MakeShareable(new FExtender);
		LevelToolbarExtender->AddToolBarExtension(
			"Settings",
			EExtensionHook::After,
			FPrefabricatorCommands::Get().LevelMenuActionList,
			FToolBarExtensionDelegate::CreateStatic(&Local::ExtendLevelToolbar)
		);

		LevelEditorModule.GetToolBarExtensibilityManager().Get()->AddExtender(LevelToolbarExtender);


		// Register asset types
		IAssetTools& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		RegisterAssetTypeAction(AssetTools, MakeShareable(new FPrefabricatorAssetTypeActions));

		// Register the asset brokers (used for asset to component mapping)
		PrefabAssetBroker = MakeShareable(new FPrefabricatorAssetBroker);
		FComponentAssetBrokerage::RegisterBroker(PrefabAssetBroker, UPrefabComponent::StaticClass(), true, true);

	}


	virtual void ShutdownModule() override {

		FLevelEditorModule* LevelEditorModule = FModuleManager::Get().GetModulePtr<FLevelEditorModule>("LevelEditor");
		if (LevelEditorModule)
		{
			if (LevelViewportExtenderHandle.IsValid())
			{
				typedef FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors DelegateType;
				LevelEditorModule->GetAllLevelViewportContextMenuExtenders().RemoveAll([=](const DelegateType& In) { return In.GetHandle() == LevelViewportExtenderHandle; });

			}

			if (LevelEditorModule->GetToolBarExtensibilityManager().IsValid()) {
				LevelEditorModule->GetToolBarExtensibilityManager().Get()->RemoveExtender(LevelToolbarExtender);
			}
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

		if (PrefabAssetBroker.IsValid()) {
			FComponentAssetBrokerage::UnregisterBroker(PrefabAssetBroker);
		}


		FPrefabEditorStyle::Shutdown();
	}

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}

	FDelegateHandle LevelViewportExtenderHandle;
	TSharedPtr<FExtender> LevelToolbarExtender;
	TArray< TSharedPtr<IAssetTypeActions> > CreatedAssetTypeActions;
	TSharedPtr<IComponentAssetBroker> PrefabAssetBroker;
};

IMPLEMENT_MODULE(FPrefabricatorEditorModule, PrefabricatorEditor)


#undef LOCTEXT_NAMESPACE


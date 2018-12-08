//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorEditorModule.h"

#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "LevelEditor.h"
#include "MultiBoxExtender.h"
#include "MultiBoxBuilder.h"
#include "PrefabEditorStyle.h"
#include "PrefabEditorCommands.h"
#include "UICommandList.h"
#include "PrefabTools.h"

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
						FExecuteAction::CreateStatic(&FPrefabTools::CreatePrefab),
						FCanExecuteAction::CreateStatic(&FPrefabTools::CanCreatePrefab)
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

		FPrefabEditorStyle::Shutdown();
	}

private:
	FDelegateHandle LevelViewportExtenderHandle;
	TSharedPtr<FExtender> LevelToolbarExtender;
};

IMPLEMENT_MODULE(FPrefabricatorEditorModule, PrefabricatorEditor)


#undef LOCTEXT_NAMESPACE


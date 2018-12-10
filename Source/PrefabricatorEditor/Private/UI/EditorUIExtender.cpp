//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "EditorUIExtender.h"

#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "PrefabEditorStyle.h"
#include "PrefabEditorTools.h"
#include "AssetToolsModule.h"
#include "LevelEditor.h"
#include "PrefabEditorCommands.h"


#define LOCTEXT_NAMESPACE "EditorUIExtender" 

void FEditorUIExtender::Extend()
{
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
					FExecuteAction::CreateStatic(&FPrefabEditorTools::CreatePrefab)
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

void FEditorUIExtender::Release()
{
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
}

#undef LOCTEXT_NAMESPACE

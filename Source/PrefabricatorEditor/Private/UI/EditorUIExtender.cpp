//$ Copyright 2015-23, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "UI/EditorUIExtender.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabTools.h"
#include "Prefab/Random/PrefabSeedLinker.h"
#include "PrefabEditorCommands.h"
#include "PrefabEditorStyle.h"
#include "PrefabricatorEditorModule.h"
#include "Utils/PrefabEditorTools.h"

#include "AssetToolsModule.h"
#include "Editor.h"
#include "EditorModeManager.h"
#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "FileHelpers.h"
#include "Framework/Commands/UICommandList.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Framework/MultiBox/MultiBoxExtender.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Framework/SlateDelegates.h"
#include "LevelEditor.h"
#include "ToolMenuEntry.h"
#include "ToolMenus.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Widgets/SNullWidget.h"

#define LOCTEXT_NAMESPACE "EditorUIExtender" 

namespace PrefabURLs {
	// TODO: move me to a configuration file
	static const FString URL_UserGuide	= "https://docs.prefabricator.dev";
	static const FString URL_Website	= "https://prefabricator.dev";
	static const FString URL_Discord	= "https://discord.gg/dRewTSU";
	static const FString URL_DevForum	= "https://forums.unrealengine.com/search?q=prefabricator&searchJSON=%7B%22keywords%22%3A%22prefabricator%22%7D";

	static const FString PATH_SAMPLE_PLATFORMER = "/Prefabricator/Samples/PF_GamePlatformer/Maps/PFGame_Platformer";
	static const FString PATH_SAMPLE_CONSTRUCTION = "/Prefabricator/Samples/PF_Construction/Maps/PFConstructionDemo";
}

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
					FExecuteAction::CreateStatic(&FPrefabTools::CreatePrefab)
				)
			);
		}

		static void LinkSelectedPrefabSeeds(APrefabSeedLinker* SeedLinker) {
			if (!SeedLinker) return;

			TArray<APrefabActor*> PrefabActors;
			{
				USelection* ActorSelectionSet = GEditor->GetSelectedActors();
				ActorSelectionSet->GetSelectedObjects(PrefabActors);
			}


			if (PrefabActors.Num() > 0) {
				for (APrefabActor* PrefabActor : PrefabActors) {
					SeedLinker->LinkedActors.AddUnique(PrefabActor);
				}
				FString NotificationText = FString::Printf(TEXT("Linked %d prefabs to Seed Linker: %s"), PrefabActors.Num(), *SeedLinker->GetName());
				FPrefabEditorTools::ShowNotification(FText::FromString(NotificationText), SNotificationItem::CS_Success);
			}
			else {
				FPrefabEditorTools::ShowNotification(LOCTEXT("NoPrefabLinkSelectedTitle", "No prefabs selected for linking"), SNotificationItem::CS_Fail);
			}
		}

		static void HandleShowToolbarPrefabSubMenu_LinkPrefabSeeds(FMenuBuilder& MenuBuilder) {
			// Grab all the prefab link actors from the scene
			UWorld* ActiveWorld = GLevelEditorModeTools().GetWorld();
			if (ActiveWorld) {
				for (TActorIterator<APrefabSeedLinker> It(ActiveWorld); It; ++It) {
					APrefabSeedLinker* SeedLinker = *It;
					if (SeedLinker) {
						FText ActorName = FText::FromString(SeedLinker->GetName());
						MenuBuilder.AddMenuEntry
						(
							ActorName,
							ActorName,
							FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.PrefabSeedLinkerActor"),
							FUIAction(FExecuteAction::CreateStatic(&Local::LinkSelectedPrefabSeeds, SeedLinker))
						);

					}
				}
			}

		}

		static void LaunchURL(FString URL) {
			FPlatformProcess::LaunchURL(*URL, nullptr, nullptr);
		}

		static void UpgradeAssets() {
			IPrefabricatorEditorModule::Get().UpgradePrefabAssets();
		}

		static void OpenSample(FString InPath) {
			// If there are any unsaved changes to the current level, see if the user wants to save those first.
			bool bPromptUserToSave = true;
			bool bSaveMapPackages = true;
			bool bSaveContentPackages = true;
			if (FEditorFileUtils::SaveDirtyPackages(bPromptUserToSave, bSaveMapPackages, bSaveContentPackages)) {
				if (FPackageName::IsValidLongPackageName(*InPath)) {
					const FString FileToOpen = FPackageName::LongPackageNameToFilename(*InPath, FPackageName::GetMapPackageExtension());
					const bool bLoadAsTemplate = false;
					const bool bShowProgress = true;
					FEditorFileUtils::LoadMap(FileToOpen, bLoadAsTemplate, bShowProgress);
				}
			}
		}

		static void HandleShowToolbarPrefabSubMenu_Community(FMenuBuilder& MenuBuilder) {
			MenuBuilder.AddMenuEntry(
				LOCTEXT("CommunityForumLabel", "Development Forum"),
				LOCTEXT("CommunityForumTooltip", "Follow along the development of the plugin and post your queries here"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.Unreal"),
				FUIAction(FExecuteAction::CreateStatic(&Local::LaunchURL, PrefabURLs::URL_DevForum))
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("CommunityDiscordLabel", "Discord Chat"),
				LOCTEXT("CommunityDiscordTooltip", "Chat with the community and post your queries here"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.Discord"),
				FUIAction(FExecuteAction::CreateStatic(&Local::LaunchURL, PrefabURLs::URL_Discord))
			);
		}

		static void HandleShowToolbarPrefabSubMenu_Advanced(FMenuBuilder& MenuBuilder) {
			MenuBuilder.AddMenuEntry(
				LOCTEXT("AdvancedUpgradeLabel", "Upgrade assets to latest version"),
				LOCTEXT("AdvancedUpgradeTooltip", "Upgrades all assets to latest version. Make a backup before proceeding"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.Unreal"),
				FUIAction(FExecuteAction::CreateStatic(&Local::UpgradeAssets))
			);

		}

		static void HandleShowToolbarPrefabSubMenu_OpenSamples(FMenuBuilder& MenuBuilder) {
			MenuBuilder.AddMenuEntry(
				LOCTEXT("SamplePlatformerLabel", "Platformer Game Demo"),
				LOCTEXT("SamplePlatformerTooltip", "Demostrates how to build a procedural platformer level with thousands of layout variations"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.Unreal"),
				FUIAction(FExecuteAction::CreateStatic(&Local::OpenSample, PrefabURLs::PATH_SAMPLE_PLATFORMER))
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("SampleConstructionLabel", "Construction System Demo"),
				LOCTEXT("SampleConstructionTooltip", "Allow your players to build their own worlds using the Construction System"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.Unreal"),
				FUIAction(FExecuteAction::CreateStatic(&Local::OpenSample, PrefabURLs::PATH_SAMPLE_CONSTRUCTION))
			);
		}

		static TSharedRef<SWidget> HandleShowToolbarPrefabMenu() {
			FMenuBuilder MenuBuilder(true, FPrefabricatorCommands::Get().LevelMenuActionList);

			MenuBuilder.BeginSection("Prefabricator-Prefabs", LOCTEXT("PrefabHeader", "Prefabs"));
			MenuBuilder.AddMenuEntry(FPrefabricatorCommands::Get().CreatePrefab);
			MenuBuilder.EndSection();

			MenuBuilder.BeginSection("Prefabricator-Randomization", LOCTEXT("RandomizationHeader", "Randomization"));

			MenuBuilder.AddSubMenu(
				LOCTEXT("MenuLinkSeeds", "Link selected Prefab Collection seeds"),
				LOCTEXT("MenuLinkSeedsTooltip", "Links the selected prefab collection seeds to an existinga Seed Linker Actor in the scene"),
				FNewMenuDelegate::CreateStatic(&Local::HandleShowToolbarPrefabSubMenu_LinkPrefabSeeds)
			);

			MenuBuilder.EndSection();

			MenuBuilder.BeginSection("Prefabricator-Help", LOCTEXT("HelpHeader", "Help / Support"));

			MenuBuilder.AddMenuEntry(
				LOCTEXT("UserGuideLabel", "User Guide"),
				LOCTEXT("UserGuideTooltip", "Detailed user guide for prefabricator"),
				FSlateIcon(FPrefabEditorStyle::Get().GetStyleSetName(), "ClassIcon.UE"),
				FUIAction(FExecuteAction::CreateStatic(&Local::LaunchURL, PrefabURLs::URL_UserGuide))
			);

			MenuBuilder.AddSubMenu(
				LOCTEXT("MenuCommunity", "Community"),
				LOCTEXT("MenuCommunityTooltip", "Get support from the developer and community"),
				FNewMenuDelegate::CreateStatic(&Local::HandleShowToolbarPrefabSubMenu_Community)
			);

			MenuBuilder.AddSubMenu(
				LOCTEXT("MenuAdvanced", "Advanced"),
				LOCTEXT("MenuAdvancedTooltip", "Advanced options"),
				FNewMenuDelegate::CreateStatic(&Local::HandleShowToolbarPrefabSubMenu_Advanced)
			);
			MenuBuilder.EndSection();

			MenuBuilder.BeginSection("Prefabricator-Samples", LOCTEXT("SamplesHeader", "Samples"));
			MenuBuilder.AddSubMenu(
				LOCTEXT("MenuOpenSamples", "Open Sample"),
				LOCTEXT("MenuOpenSamplesTooltip", "Open a sample map"),
				FNewMenuDelegate::CreateStatic(&Local::HandleShowToolbarPrefabSubMenu_OpenSamples)
			);
			MenuBuilder.EndSection();

			return MenuBuilder.MakeWidget();
		}

		static void ExtendLevelToolbar(FToolBarBuilder& ToolbarBuilder) {
			
			ToolbarBuilder.AddComboButton(FUIAction(),
				FOnGetContent::CreateStatic(&Local::HandleShowToolbarPrefabMenu),
				LOCTEXT("PrefabToolbarButtonText", "Prefabricator"),
				LOCTEXT("PrefabToolbarButtonTooltip", "Show Prefabricator actions"),
				FSlateIcon(FPrefabEditorStyle::GetStyleSetName(), TEXT("Prefabricator.CreatePrefab")));
		}
	};

	FLevelEditorModule& LevelEditorModule = FModuleManager::Get().LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	auto& MenuExtenders = LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	MenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::CreateStatic(&Local::OnExtendLevelEditorActorContextMenu));
	LevelViewportExtenderHandle = MenuExtenders.Last().GetHandle();

	UToolMenu* AssetsToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.AssetsToolBar");
	if (AssetsToolBar) {
		FToolMenuSection& Section = AssetsToolBar->AddSection("Prefabricator");
		const FToolMenuEntry LaunchPadEntry = FToolMenuEntry::InitComboButton("Prefabricator",
																				FUIAction(),
																				FOnGetContent::CreateStatic(&Local::HandleShowToolbarPrefabMenu),
																				LOCTEXT("PrefabricatorToolbarButtonText", "Prefabricator"),
																				LOCTEXT("DAToolbarButtonTooltip", "Prefabricator Menu"),
																				FSlateIcon(FPrefabEditorStyle::GetStyleSetName(), TEXT("Prefabricator.CreatePrefab")));
		Section.AddEntry(LaunchPadEntry);
	}
	
	
}

void FEditorUIExtender::Release()
{
	FLevelEditorModule* LevelEditorModule = FModuleManager::Get().GetModulePtr<FLevelEditorModule>("LevelEditor");
	if (LevelEditorModule)
	{
		if (LevelViewportExtenderHandle.IsValid())
		{
			typedef FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors DelegateType;
			LevelEditorModule->GetAllLevelViewportContextMenuExtenders().RemoveAll([this](const DelegateType& In) { return In.GetHandle() == LevelViewportExtenderHandle; });
		}

		if (LevelEditorModule->GetToolBarExtensibilityManager().IsValid()) {
			LevelEditorModule->GetToolBarExtensibilityManager().Get()->RemoveExtender(LevelToolbarExtender);
		}
	}
}

#undef LOCTEXT_NAMESPACE


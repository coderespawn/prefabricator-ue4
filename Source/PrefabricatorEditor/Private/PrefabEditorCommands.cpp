//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabEditorCommands.h"
#include "PrefabEditorStyle.h"
#include "Commands.h"
#include "UICommandList.h"
#include "UIAction.h"
#include "PrefabEditorTools.h"
#include "Object.h"


#define LOCTEXT_NAMESPACE "ContentBrowser"

FPrefabricatorCommands::FPrefabricatorCommands() : TCommands<FPrefabricatorCommands>(
	TEXT("Prefabricator"),
	NSLOCTEXT("Prefabricator", "Prefabricator", "Prefab Architect"),
	NAME_None,
	FPrefabEditorStyle::GetStyleSetName())
{
}

void FPrefabricatorCommands::RegisterCommands() {
	UI_COMMAND(CreatePrefab, "Create Prefab", "Create a new prefab from selection", EUserInterfaceActionType::Button, FInputChord(EKeys::Enter));

	LevelMenuActionList = MakeShareable(new FUICommandList);

	LevelMenuActionList->MapAction(
		CreatePrefab,
		FExecuteAction::CreateStatic(&FPrefabEditorTools::CreatePrefab),
		FCanExecuteAction::CreateStatic(&FPrefabEditorTools::CanCreatePrefab)
	);
}


#undef LOCTEXT_NAMESPACE


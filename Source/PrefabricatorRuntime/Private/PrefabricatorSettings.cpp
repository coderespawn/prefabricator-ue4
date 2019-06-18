// Fill out your copyright notice in the Description page of Project Settings.

#include "PrefabricatorSettings.h"

UPrefabricatorSettings::UPrefabricatorSettings()
{
	//Do not change default behavior.
	bAllowDynamicUpdate = true;
}

/** Gets the settings container name for the settings, either Project or Editor */
FName UPrefabricatorSettings::GetContainerName() const
{
	return TEXT("Project");
}
/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
FName UPrefabricatorSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}
/** The unique name for your section of settings, uses the class's FName. */
FName UPrefabricatorSettings::GetSectionName() const
{
	return TEXT("Prefabricator");
}

#if WITH_EDITOR
/** Gets the section text, uses the classes DisplayName by default. */
FText UPrefabricatorSettings::GetSectionText() const
{
	return FText::FromString("Prefabricator");
}
/** Gets the description for the section, uses the classes ToolTip by default. */
FText UPrefabricatorSettings::GetSectionDescription() const
{
	return FText::FromString("Prefabricator Settings");
}
#endif
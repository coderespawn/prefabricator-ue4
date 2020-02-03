//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabEditorStyle.h"

#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/SlateTypes.h"

#define BASE_PATH FPaths::GamePluginsDir() / "Prefabricator/Content"
#define IMAGE_PLUGIN_BRUSH( RelativePath, ... ) FSlateImageBrush( FPrefabEditorStyle::InContent( RelativePath, ".png" ), __VA_ARGS__ )
#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style.RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style.RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style.RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define DEFAULT_FONT(...) FCoreStyle::GetDefaultFontStyle(__VA_ARGS__)
#define ICON_FONT(...) FSlateFontInfo(Style.RootToContentDir("Fonts/FontAwesome", TEXT(".ttf")), __VA_ARGS__)


TSharedPtr< FSlateStyleSet > FPrefabEditorStyle::StyleInstance = nullptr;

FString FPrefabEditorStyle::InContent(const FString& RelativePath, const ANSICHAR* Extension)
{
	static FString ContentDir = IPluginManager::Get().FindPlugin(TEXT("Prefabricator"))->GetContentDir();
	return (ContentDir / RelativePath) + Extension;
}

void FPrefabEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FPrefabEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FPrefabEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("PrefabricatorStyle"));
	return StyleSetName;
}

TSharedRef< class FSlateStyleSet > FPrefabEditorStyle::Create()
{
	const FVector2D Icon16x16(16.0f, 16.0f);
	const FVector2D Icon20x20(20.0f, 20.0f);
	const FVector2D Icon23x23(23.0f, 23.0f);
	const FVector2D Icon24x24(24.0f, 24.0f);
	const FVector2D Icon32x32(32.0f, 32.0f);
	const FVector2D Icon36x36(36.0f, 36.0f);
	const FVector2D Icon40x40(40.0f, 40.0f);
	const FVector2D Icon48x48(48.0f, 48.0f);

	TSharedRef<FSlateStyleSet> StyleRef = MakeShareable(new FSlateStyleSet(FPrefabEditorStyle::GetStyleSetName()));
	StyleRef->SetContentRoot(FPaths::EngineContentDir() / TEXT("Editor/Slate"));
	StyleRef->SetCoreContentRoot(FPaths::EngineContentDir() / TEXT("Slate"));

	FSlateStyleSet& Style = StyleRef.Get();
	// Generic styles
	{
		Style.Set("Prefabricator.ContextMenu.Icon", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_Prefab_16", Icon16x16));
		Style.Set("Prefabricator.CreatePrefab", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_Prefab_48", Icon48x48));

		Style.Set("ClassIcon.PrefabActor", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_Prefab_16", Icon16x16));
		Style.Set("ClassIcon.PrefabRandomizerActor", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_randomizer_16", Icon16x16));
		Style.Set("ClassIcon.PrefabSeedLinkerActor", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_linker_16", Icon16x16));

		Style.Set("ClassIcon.Unreal", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_ue_16", Icon16x16));
		Style.Set("ClassIcon.Discord", new IMAGE_PLUGIN_BRUSH("PrefabTool/Icons/Icon_discord_16", Icon16x16));

	}



	return StyleRef;
}


#undef IMAGE_PLUGIN_BRUSH

const ISlateStyle& FPrefabEditorStyle::Get()
{
	return *StyleInstance;
}


//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

namespace UnrealBuildTool.Rules
{
	public class ConstructionSystemEditor : ModuleRules
	{
		public ConstructionSystemEditor(ReadOnlyTargetRules Target) : base(Target)
        {
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
                }
				);

			PrivateIncludePaths.AddRange(
				new string[] {
					// ... add other private include paths required here ...
				}
				);

            PrivateIncludePathModuleNames.AddRange(
                new string[] {
                "Settings",
                "AssetTools",
            }
            );

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "EditorStyle",
                    "UnrealEd",
                    "AdvancedPreviewScene",
                    "Kismet",
                    "PlacementMode",
                    "EditorWidgets",
                }
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "InputCore",
                    "Slate",
                    "SlateCore",
				    "RenderCore",
                    "PropertyEditor",
                    "WorkspaceMenuStructure",
                    "LevelEditor",
				    "EditorStyle",
				    "ContentBrowser",
                    "Projects",
                    "PrefabricatorRuntime",
                    "PrefabricatorEditor",
                    "ConstructionSystemRuntime",
					// ... add private dependencies that you statically link with here ...
				}
				);

			DynamicallyLoadedModuleNames.AddRange(
				new string[]
				{
					// ... add any modules that your module loads dynamically here ...
                }
				);
		}
	}
}

//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

namespace UnrealBuildTool.Rules
{
	public class PrefabricatorRuntime : ModuleRules
	{
		public PrefabricatorRuntime(ReadOnlyTargetRules Target) : base(Target)
        {
            bUseUnity = false;
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PublicIncludePaths.AddRange(
				new string[] {
					// ... add public include paths required here ...
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
				    "Engine",
                    "PropertyPath",
					// ... add other public dependencies that you statically link with here ...
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
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

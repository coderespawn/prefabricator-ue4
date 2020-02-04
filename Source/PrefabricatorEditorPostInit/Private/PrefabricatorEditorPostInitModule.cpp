//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorEditorPostInitModule.h"

#include "Prefab/PrefabComponent.h"
#include "Prefab/Random/PrefabSeedLinker.h"
#include "Visualizers/PrefabComponentVisualizer.h"
#include "Visualizers/PrefabSeedLinkerVisualizer.h"

#include "Editor/UnrealEdEngine.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "UnrealEdGlobals.h"

#define LOCTEXT_NAMESPACE "PrefabricatorEditorModule" 



class FPrefabricatorEditorPostInitModule : public IPrefabricatorEditorPostInitModule
{
	virtual void StartupModule() override 
	{
		// Register component visualizers
		RegisterComponentVisualizers();
	}

	virtual void ShutdownModule() override {
		UnregisterComponentVisualizers();

	}

private:
	void RegisterComponentVisualizers() {
		if (GUnrealEd) {
			PrefabComponentClassName = UPrefabComponent::StaticClass()->GetFName();
			PrefabSeedLinkerComponentClassName = UPrefabSeedLinkerComponent::StaticClass()->GetFName();

			RegisterVisualizer(*GUnrealEd, PrefabComponentClassName, MakeShared<FPrefabComponentVisualizer>());
			RegisterVisualizer(*GUnrealEd, PrefabSeedLinkerComponentClassName, MakeShared<FPrefabSeedLinkerVisualizer>());
		}
	}

	void RegisterVisualizer(UUnrealEdEngine& UnrealEdEngine, const FName& ComponentClassName, const TSharedRef<FComponentVisualizer>& Visualizer)
	{
		UnrealEdEngine.RegisterComponentVisualizer(ComponentClassName, Visualizer);
		Visualizer->OnRegister();
	}

	void UnregisterComponentVisualizers() {
		if (GUnrealEd) {
			GUnrealEd->UnregisterComponentVisualizer(PrefabComponentClassName);
			GUnrealEd->UnregisterComponentVisualizer(PrefabSeedLinkerComponentClassName);
		}
	}

	FName PrefabComponentClassName;
	FName PrefabSeedLinkerComponentClassName;
};

IMPLEMENT_MODULE(FPrefabricatorEditorPostInitModule, PrefabricatorEditorPostInit)


#undef LOCTEXT_NAMESPACE


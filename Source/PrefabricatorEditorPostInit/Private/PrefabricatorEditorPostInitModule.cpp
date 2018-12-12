//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorEditorPostInitModule.h"
#include "IAssetTools.h"
#include "IAssetTypeActions.h"
#include "PrefabComponent.h"
#include "PrefabComponentVisualizer.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"


#define LOCTEXT_NAMESPACE "DungeonArchitectEditorModule" 



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
			RegisterVisualizer(*GUnrealEd, PrefabComponentClassName, MakeShared<FPrefabComponentVisualizer>());
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
		}
	}

	FName PrefabComponentClassName;
};

IMPLEMENT_MODULE(FPrefabricatorEditorPostInitModule, PrefabricatorEditorPostInit)


#undef LOCTEXT_NAMESPACE


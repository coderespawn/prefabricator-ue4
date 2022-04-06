//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemEditorModule.h"

#include "IAssetTools.h"

#define LOCTEXT_NAMESPACE "PrefabricatorEditorModule" 



class FConstructionSystemEditorModule : public IConstructionSystemEditorModule
{
	virtual void StartupModule() override 
	{
	}

	virtual void ShutdownModule() override {


	}

private:
	void RegisterAssetTypeAction(IAssetTools& AssetTools, TSharedRef<IAssetTypeActions> Action)
	{
		AssetTools.RegisterAssetTypeActions(Action);
		CreatedAssetTypeActions.Add(Action);
	}
	
	TArray<TSharedPtr<IAssetTypeActions>> CreatedAssetTypeActions;
};

IMPLEMENT_MODULE(FConstructionSystemEditorModule, ConstructionSystemEditor)


#undef LOCTEXT_NAMESPACE


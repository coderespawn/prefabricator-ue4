//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Framework/Commands/Commands.h"
#include "Framework/Commands/UICommandInfo.h"

class PREFABRICATOREDITOR_API FPrefabricatorCommands : public TCommands<FPrefabricatorCommands>
{
public:
	FPrefabricatorCommands();

	virtual void RegisterCommands() override;

public:

	TSharedPtr<FUICommandInfo> CreatePrefab;
	TSharedPtr<class FUICommandList> LevelMenuActionList;

};



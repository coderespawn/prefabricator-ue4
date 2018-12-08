//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Commands.h"
#include "UICommandInfo.h"

class PREFABRICATOREDITOR_API FPrefabricatorCommands : public TCommands<FPrefabricatorCommands>
{
public:
	FPrefabricatorCommands();

	virtual void RegisterCommands() override;

public:

	TSharedPtr<FUICommandInfo> CreatePrefab;
	TSharedPtr<class FUICommandList> LevelMenuActionList;

};



//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "LevelEditor.h"

class PREFABRICATOREDITOR_API FEditorUIExtender {
public:
	void Extend();
	void Release();

private:
	FDelegateHandle LevelViewportExtenderHandle;
	TSharedPtr<class FExtender> LevelToolbarExtender;
};


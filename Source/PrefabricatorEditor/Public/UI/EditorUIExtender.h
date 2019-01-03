//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "LevelEditor.h"

class FEditorUIExtender {
public:
	void Extend();
	void Release();

private:
	FDelegateHandle LevelViewportExtenderHandle;
	TSharedPtr<class FExtender> LevelToolbarExtender;
};

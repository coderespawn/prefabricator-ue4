//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/Tools/ConstructionSystemTool.h"

void UConstructionSystemTool::InitializeTool(APawn* Owner)
{
	if (Owner && Owner->InputComponent) {
		RegisterInputCallbacks(Owner->InputComponent);
	}
}

void UConstructionSystemTool::DestroyTool(APawn* Owner)
{
	if (Owner && Owner->InputComponent) {
		UnregisterInputCallbacks(Owner->InputComponent);
	}
}

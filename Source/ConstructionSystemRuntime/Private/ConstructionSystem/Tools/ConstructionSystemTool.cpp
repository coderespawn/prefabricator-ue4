//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/Tools/ConstructionSystemTool.h"
#include "GameFramework/Pawn.h"
#include "ConstructionSystemComponent.h"

void UConstructionSystemTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	APawn* Pawn = Cast<APawn>(ConstructionComponent ? ConstructionComponent->GetOwner() : nullptr);
	if (Pawn && Pawn->InputComponent) {
		RegisterInputCallbacks(Pawn->InputComponent);
	}
}

void UConstructionSystemTool::DestroyTool(UConstructionSystemComponent* ConstructionComponent)
{
	APawn* Pawn = Cast<APawn>(ConstructionComponent ? ConstructionComponent->GetOwner() : nullptr);
	if (Pawn && Pawn->InputComponent) {
		UnregisterInputCallbacks(Pawn->InputComponent);
	}
}

void UConstructionSystemTool::OnToolEnable(UConstructionSystemComponent* ConstructionComponent)
{
	bToolEnabled = true;
}

void UConstructionSystemTool::OnToolDisable(UConstructionSystemComponent* ConstructionComponent)
{
	bToolEnabled = false;
}

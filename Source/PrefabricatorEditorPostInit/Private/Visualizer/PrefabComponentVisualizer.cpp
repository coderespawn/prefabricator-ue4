//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Visualizers/PrefabComponentVisualizer.h"

#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "GameFramework/Actor.h"
#include "SceneManagement.h"

void FPrefabComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UPrefabComponent* PrefabComponent = Cast<const UPrefabComponent>(Component);
	if (!PrefabComponent) return;
	
	AActor* Parent = PrefabComponent->GetOwner();
	if (!Parent) return;

	FBox Bounds = FPrefabTools::GetPrefabBounds(Parent);
	Bounds = Bounds.ExpandBy(2);

	const float Thickness = 0;
	const FMatrix LocalToWorld = FMatrix::Identity;
	DrawWireBox(PDI, LocalToWorld, Bounds, FLinearColor::Green, SDPG_Foreground, Thickness);
}


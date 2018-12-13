//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Visualizers/PrefabComponentVisualizer.h"

#include "Prefab/PrefabComponent.h"

#include "GameFramework/Actor.h"
#include "SceneManagement.h"

void FPrefabComponentVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UPrefabComponent* PrefabComponent = Cast<const UPrefabComponent>(Component);
	if (!PrefabComponent) return;
	
	AActor* Parent = PrefabComponent->GetOwner();
	if (!Parent) return;

	TArray<AActor*> AttachedActors;
	Parent->GetAttachedActors(AttachedActors);

	FBox Bounds(EForceInit::ForceInit); // = Parent->GetComponentsBoundingBox(true);
	for (AActor* AttachedActor : AttachedActors) {
		Bounds += AttachedActor->GetComponentsBoundingBox(true);
	}

	Bounds = Bounds.ExpandBy(2);

	const FMatrix LocalToWorld = FMatrix::Identity;
	DrawWireBox(PDI, LocalToWorld, Bounds, FLinearColor::Green, SDPG_World);
}


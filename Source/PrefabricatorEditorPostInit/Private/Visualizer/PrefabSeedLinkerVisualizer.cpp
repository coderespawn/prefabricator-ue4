//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Visualizers/PrefabSeedLinkerVisualizer.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"
#include "Prefab/Random/PrefabSeedLinker.h"

#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "SceneManagement.h"

void FPrefabSeedLinkerVisualizer::DrawVisualization(const UActorComponent* Component, const FSceneView* View, FPrimitiveDrawInterface* PDI)
{
	const UPrefabSeedLinkerComponent* SeedLinkerComponent = Cast<const UPrefabSeedLinkerComponent>(Component);
	if (!SeedLinkerComponent) return;

	APrefabSeedLinker* SeedLinkerActor = Cast<APrefabSeedLinker>(SeedLinkerComponent->GetOwner());
	if (!SeedLinkerActor) return;

	FVector StartLocation = SeedLinkerComponent->GetComponentLocation();
	for (TWeakObjectPtr<APrefabActor> LinkedActor : SeedLinkerActor->LinkedActors) {
		if (LinkedActor.IsValid()) {
			FBoxSphereBounds Bounds = LinkedActor->PrefabComponent->Bounds;
			FVector EndLocation = Bounds.Origin;

			PDI->DrawLine(StartLocation, EndLocation, FLinearColor::Red, SDPG_Foreground);
			const FMatrix LocalToWorld = FMatrix::Identity;
			DrawWireBox(PDI, LocalToWorld, Bounds.GetBox(), FLinearColor::Red, SDPG_Foreground);
		}
	}
}


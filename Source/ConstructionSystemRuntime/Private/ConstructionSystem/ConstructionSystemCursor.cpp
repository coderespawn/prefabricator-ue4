//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/ConstructionSystemCursor.h"
#include "PrefabTools.h"
#include "PrefabActor.h"
#include "Components/PrimitiveComponent.h"
#include "PrefabricatorAsset.h"
#include "PrefabComponent.h"
#include "PrefabricatorFunctionLibrary.h"

void UConstructionSystemCursor::RecreateCursor(UWorld* InWorld, UPrefabricatorAssetInterface* InActivePrefabAsset)
{
	DestroyCursor();

	if (InWorld && InActivePrefabAsset) {
		CursorGhostActor = InWorld->SpawnActor<APrefabActor>();
		CursorGhostActor->PrefabComponent->PrefabAssetInterface = InActivePrefabAsset;


		FRandomStream RandomStream(CursorSeed);
		UPrefabricatorBlueprintLibrary::RandomizePrefab(CursorGhostActor, RandomStream);
		CursorGhostActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);

		FPrefabTools::IterateChildrenRecursive(CursorGhostActor, [this](AActor* ChildActor) {
			for (UActorComponent* Component : ChildActor->GetComponents()) {
				if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component)) {
					// Disable collision
					Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);

					// Set cursor material
					if (CursorMaterial) {
						int32 NumMaterials = Primitive->GetNumMaterials();
						for (int ElementIndex = 0; ElementIndex < NumMaterials; ElementIndex++) {
							Primitive->SetMaterial(ElementIndex, CursorMaterial);
						}
					}
				}
			}
		});

	}
}

void UConstructionSystemCursor::DestroyCursor()
{
	if (CursorGhostActor) {
		CursorGhostActor->Destroy();
		CursorGhostActor = nullptr;
	}
}

void UConstructionSystemCursor::SetVisiblity(bool bVisible)
{
	if (CursorGhostActor) {
		CursorGhostActor->SetActorHiddenInGame(!bVisible);
	}
}

void UConstructionSystemCursor::SetTransform(const FVector& Location, const FVector& Normal)
{
	if (CursorGhostActor) {
		CursorGhostActor->SetActorLocation(Location);
	}
}

bool UConstructionSystemCursor::GetCursorTransform(FTransform& OutTransform) const
{
	if (CursorGhostActor) {
		OutTransform = CursorGhostActor->GetActorTransform();
		return true;
	}
	return false;
}

//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/ConstructionSystemCursor.h"
#include "PrefabTools.h"
#include "PrefabActor.h"
#include "Components/PrimitiveComponent.h"
#include "PrefabricatorAsset.h"
#include "PrefabComponent.h"
#include "PrefabricatorFunctionLibrary.h"
#include "ConstructionSystemSnap.h"

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

				if (UPrefabricatorConstructionSnapComponent* SnapComponent = Cast<UPrefabricatorConstructionSnapComponent>(Component)) {
					SnapComponents.Add(SnapComponent);
				}
			}
		});
	}
}

void UConstructionSystemCursor::DestroyCursor()
{
	if (CursorGhostActor) {
		SnapComponents.Reset();
		CursorGhostActor->Destroy();
		ActiveSnapComponentIndex = 0;
		CursorGhostActor = nullptr;
	}
}

void UConstructionSystemCursor::SetVisiblity(bool bVisible)
{
	if (CursorGhostActor) {
		FPrefabTools::IterateChildrenRecursive(CursorGhostActor, [bVisible](AActor* ChildActor) {
			ChildActor->SetActorHiddenInGame(!bVisible);
		});
	}
}

void UConstructionSystemCursor::SetTransform(const FTransform& InTransform)
{
	if (CursorGhostActor) {
		CursorGhostActor->SetActorTransform(InTransform);
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

void UConstructionSystemCursor::MoveToNextSnapComponent()
{
	if (SnapComponents.Num() > 0) {
		ActiveSnapComponentIndex = (ActiveSnapComponentIndex + 1) % SnapComponents.Num();
	}
}

void UConstructionSystemCursor::MoveToPrevSnapComponent()
{
	if (SnapComponents.Num() > 0) {
		ActiveSnapComponentIndex = (ActiveSnapComponentIndex - 1);
		if (ActiveSnapComponentIndex < 0) {
			ActiveSnapComponentIndex += SnapComponents.Num();
		}
	}
}

UPrefabricatorConstructionSnapComponent* UConstructionSystemCursor::GetActiveSnapComponent()
{
	if (ActiveSnapComponentIndex < 0 || ActiveSnapComponentIndex >= SnapComponents.Num()) {
		return nullptr;
	}
	return SnapComponents[ActiveSnapComponentIndex];
}

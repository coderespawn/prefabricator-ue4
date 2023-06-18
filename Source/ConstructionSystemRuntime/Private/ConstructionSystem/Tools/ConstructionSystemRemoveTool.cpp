//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/Tools/ConstructionSystemRemoveTool.h"

#include "ConstructionSystem/ConstructionSystemCursor.h"
#include "ConstructionSystem/ConstructionSystemSnap.h"
#include "ConstructionSystemComponent.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabTools.h"
#include "Utils/ConstructionSystemDefs.h"
#include "Utils/ConstructionSystemUtils.h"

#include "CollisionQueryParams.h"
#include "DrawDebugHelpers.h"
#include "Engine/World.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"

void UConstructionSystemRemoveTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::InitializeTool(ConstructionComponent);

	PrefabSnapChannel = FConstructionSystemUtils::FindPrefabSnapChannel();
}

void UConstructionSystemRemoveTool::DestroyTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::DestroyTool(ConstructionComponent);
}

void UConstructionSystemRemoveTool::OnToolEnable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolEnable(ConstructionComponent);

}

void UConstructionSystemRemoveTool::OnToolDisable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolDisable(ConstructionComponent);

}

void UConstructionSystemRemoveTool::Update(UConstructionSystemComponent* ConstructionComponent)
{

	if (!ConstructionComponent) return;

	UWorld* World = ConstructionComponent->GetWorld();
	if (!World) return;

	APlayerController* PlayerController = Cast<APlayerController>(ConstructionComponent->GetOwner());

	bCursorFoundHit = false;
	FocusedActor = nullptr;

	if (PlayerController) {
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
		FVector CameraDirection = ViewRotation.RotateVector(FVector::ForwardVector);
		FVector StartLocation = ViewLocation + CameraDirection * ConstructionComponent->TraceStartDistance;
		FVector EndLocation = ViewLocation + CameraDirection * (TraceDistance + ConstructionComponent->TraceStartDistance);


		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(PlayerController->GetPawn());

		FHitResult Hit;
		if (World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, PrefabSnapChannel, QueryParams, ResponseParams)) {
			UPrefabricatorConstructionSnapComponent* SnapComponent = Cast<UPrefabricatorConstructionSnapComponent>(Hit.GetComponent());
			if (SnapComponent) {
				// find the prefab actor
				AActor* CurrentActor = Hit.GetActor();
				while (APrefabActor* ParentPrefab = Cast<APrefabActor>(CurrentActor->GetAttachParentActor())) {
					FocusedActor = ParentPrefab;
					CurrentActor = ParentPrefab;
					bCursorFoundHit = true;

				}
				DrawDebugPoint(World, Hit.ImpactPoint, 20, FColor::Blue);
				FBox Bounds = FPrefabTools::GetPrefabBounds(FocusedActor.Get());
				DrawDebugBox(World, Bounds.GetCenter(), Bounds.GetExtent(), FColor::Red);
			}
		}
	}

}

void UConstructionSystemRemoveTool::HandleInput_RemoveAtCursor()
{
	if (!bInputPaused) {
		RemoveAtCursor();
	}
}

void UConstructionSystemRemoveTool::RemoveAtCursor()
{
	if (bToolEnabled && bCursorFoundHit && FocusedActor.IsValid()) {
		FocusedActor->Destroy();
		FocusedActor = nullptr;
		bCursorFoundHit = false;
	}
}

void UConstructionSystemRemoveTool::RegisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::RegisterInputCallbacks(InputComponent);

	InputBindings.RemoveAtCursor = InputComponent->BindAction("CSRemoveAtCursor", IE_Pressed, this, &UConstructionSystemRemoveTool::HandleInput_RemoveAtCursor);

}

void UConstructionSystemRemoveTool::UnregisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::UnregisterInputCallbacks(InputComponent);

	InputBindings.RemoveAtCursor.ActionDelegate.Unbind();
	InputBindings = FCSRemoveToolInputBindings();
}


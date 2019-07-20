//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemRemoveTool.h"
#include "ConstructionSystemCursor.h"
#include "ConstructionSystemComponent.h"
#include "ConstructionSystemUtils.h"
#include "GameFramework/PlayerController.h"
#include "ConstructionSystemDefs.h"
#include "GameFramework/Pawn.h"
#include "CollisionQueryParams.h"
#include "Engine/World.h"
#include "PrefabActor.h"
#include "DrawDebugHelpers.h"
#include "PrefabTools.h"
#include "ConstructionSystemSnap.h"


void UConstructionSystemRemoveTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::InitializeTool(ConstructionComponent);

	Cursor = NewObject<UConstructionSystemCursor>(this, "Cursor");
	Cursor->SetCursorMaterial(ConstructionComponent->CursorMaterial);
	Cursor->SetCursorInvalidMaterial(ConstructionComponent->CursorInvalidMaterial);

	PrefabSnapChannel = FConstructionSystemUtils::FindPrefabSnapChannel();
}

void UConstructionSystemRemoveTool::DestroyTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::DestroyTool(ConstructionComponent);
	if (Cursor) {
		Cursor->DestroyCursor();
	}
}

void UConstructionSystemRemoveTool::OnToolEnable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolEnable(ConstructionComponent);
	if (Cursor) {
		Cursor->SetVisiblity(EConstructionSystemCursorVisiblity::Visible);
	}
}

void UConstructionSystemRemoveTool::OnToolDisable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolDisable(ConstructionComponent);

	if (Cursor) {
		Cursor->SetVisiblity(EConstructionSystemCursorVisiblity::Hidden);
	}
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
		FVector StartLocation = ViewLocation + CameraDirection * FConstructionSystemConstants::BuildToolSweepRadius;
		FVector EndLocation = ViewLocation + CameraDirection * (TraceDistance + FConstructionSystemConstants::BuildToolSweepRadius);


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

void UConstructionSystemRemoveTool::RemoveAtCursor()
{
	if (bCursorFoundHit && FocusedActor.IsValid()) {
		FocusedActor->Destroy();
		FocusedActor = nullptr;
		bCursorFoundHit = false;
	}
}

void UConstructionSystemRemoveTool::RegisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::RegisterInputCallbacks(InputComponent);

	InputBindings.RemoveAtCursor = InputComponent->BindAction("CSRemoveAtCursor", IE_Pressed, this, &UConstructionSystemRemoveTool::RemoveAtCursor);

}

void UConstructionSystemRemoveTool::UnregisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::UnregisterInputCallbacks(InputComponent);

	InputBindings.RemoveAtCursor.ActionDelegate.Unbind();
	InputBindings = FCSRemoveToolInputBindings();
}

//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/Tools/ConstructionSystemBuildTool.h"
#include "PrefabricatorAsset.h"
#include "ConstructionSystemCursor.h"
#include "Materials/MaterialInterface.h"
#include "ConstructionSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "CollisionQueryParams.h"
#include "PrefabTools.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PrefabActor.h"
#include "PrefabricatorFunctionLibrary.h"
#include "PrefabComponent.h"

void UConstructionSystemBuildTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::InitializeTool(ConstructionComponent);

	Cursor = NewObject<UConstructionSystemCursor>(this, "Cursor");
	Cursor->SetCursorMaterial(ConstructionComponent->CursorMaterial);
}

void UConstructionSystemBuildTool::DestroyTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::DestroyTool(ConstructionComponent);
	if (Cursor) {
		Cursor->DestroyCursor();
	}
}

void UConstructionSystemBuildTool::OnToolEnable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolEnable(ConstructionComponent);
	if (Cursor) {
		Cursor->SetVisiblity(true);
	}
}

void UConstructionSystemBuildTool::OnToolDisable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolDisable(ConstructionComponent);

	if (Cursor) {
		Cursor->SetVisiblity(false);
	}
}

void UConstructionSystemBuildTool::Update(UConstructionSystemComponent* ConstructionComponent)
{
	if (!ConstructionComponent) return;

	APawn* Owner = Cast<APawn>(ConstructionComponent->GetOwner());
	APlayerController* PlayerController = Owner ? Owner->GetController<APlayerController>() : nullptr;
	if (PlayerController) {
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		FVector StartLocation = ViewLocation;
		FVector CameraDirection = ViewRotation.RotateVector(FVector::ForwardVector);
		FVector EndLocation = StartLocation + CameraDirection * TraceDistance;

		FHitResult Hit;
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(ConstructionComponent->GetOwner());
		if (Cursor->GetCursorGhostActor()) {
			FPrefabTools::IterateChildrenRecursive(Cursor->GetCursorGhostActor(), [&QueryParams](AActor* ChildCursorActor) {
				QueryParams.AddIgnoredActor(ChildCursorActor);
			});
		}
		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;
		UWorld* World = ConstructionComponent->GetWorld();

		if (World && World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams, ResponseParams)) {
			Cursor->SetTransform(Hit.ImpactPoint, Hit.Normal);
		}

		// Draw debug info
		DrawDebugPoint(World, Hit.ImpactPoint, 20, FColor::Red);
	}
}

void UConstructionSystemBuildTool::RegisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::RegisterInputCallbacks(InputComponent);

}

void UConstructionSystemBuildTool::UnregisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::UnregisterInputCallbacks(InputComponent);

}


void UConstructionSystemBuildTool::SetActivePrefab(UPrefabricatorAssetInterface* InActivePrefabAsset)
{
	ActivePrefabAsset = InActivePrefabAsset;
	Cursor->RecreateCursor(GetWorld(), InActivePrefabAsset);
}

void UConstructionSystemBuildTool::ConstructAtCursor()
{
	UConstructionSystemComponent* ConstructionComponent = Cast<UConstructionSystemComponent>(GetOuter());
	if (ConstructionComponent && bToolEnabled) {
		UWorld* World = ConstructionComponent->GetWorld();
		if (!World) {
			return;
		}

		if (World && ActivePrefabAsset) {
			FTransform Transform;
			if (Cursor->GetCursorTransform(Transform)) {
				APrefabActor* SpawnedPrefab = GetWorld()->SpawnActor<APrefabActor>(APrefabActor::StaticClass(), Transform);
				SpawnedPrefab->PrefabComponent->PrefabAssetInterface = ActivePrefabAsset;

				FRandomStream RandomStream(Cursor->GetCursorSeed());
				UPrefabricatorBlueprintLibrary::RandomizePrefab(SpawnedPrefab, RandomStream);
			}
		}
	}
}

void UConstructionSystemBuildTool::CursorMoveNext()
{
	Cursor->IncrementSeed();
	Cursor->RecreateCursor(GetWorld(), ActivePrefabAsset);
}

void UConstructionSystemBuildTool::CursorMovePrev()
{
	Cursor->DecrementSeed();
	Cursor->RecreateCursor(GetWorld(), ActivePrefabAsset);
}

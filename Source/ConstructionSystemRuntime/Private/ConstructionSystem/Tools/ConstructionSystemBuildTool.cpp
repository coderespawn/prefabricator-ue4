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
#include "ConstructionSystemDefs.h"
#include "ConstructionSystemUtils.h"
#include "ConstructionSystemSnap.h"

namespace {
}

void UConstructionSystemBuildTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::InitializeTool(ConstructionComponent);

	Cursor = NewObject<UConstructionSystemCursor>(this, "Cursor");
	Cursor->SetCursorMaterial(ConstructionComponent->CursorMaterial);

	PrefabSnapChannel = FConstructionSystemUtils::FindPrefabSnapChannel();
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

	UWorld* World = ConstructionComponent->GetWorld();
	if (!World) return;

	APawn* Owner = Cast<APawn>(ConstructionComponent->GetOwner());
	APlayerController* PlayerController = Owner ? Owner->GetController<APlayerController>() : nullptr;
	if (PlayerController) {
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		FVector StartLocation = ViewLocation;
		FVector CameraDirection = ViewRotation.RotateVector(FVector::ForwardVector);
		FVector EndLocation = StartLocation + CameraDirection * TraceDistance;

		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(ConstructionComponent->GetOwner());
		if (Cursor->GetCursorGhostActor()) {
			FPrefabTools::IterateChildrenRecursive(Cursor->GetCursorGhostActor(), [&QueryParams](AActor* ChildCursorActor) {
				QueryParams.AddIgnoredActor(ChildCursorActor);
			});
		}

		FHitResult Hit;
		bool bFoundHit = false;
		bool bHitSnapChannel = false;
		
		FCollisionShape SweepShape = FCollisionShape::MakeSphere(FConstructionSystemConstants::BuildToolSweepRadius);
		if (World->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, PrefabSnapChannel, SweepShape, QueryParams, ResponseParams)) {
			bFoundHit = true;
			bHitSnapChannel = true;
		}
		else if (World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams, ResponseParams)) {
			// We did not hit anything. Trace in the static world
			bFoundHit = true;
		}

		if (bFoundHit) {
			FVector CursorLocation;
			FQuat CursorRotation;
			if (bHitSnapChannel) {
				// Snap the cursor
				CursorLocation = Hit.ImpactPoint;
				CursorRotation = FQuat::Identity;
				UPrefabricatorConstructionSnapComponent* CursorSnap = Cursor->GetActiveSnapComponent();
				UPrefabricatorConstructionSnapComponent* HitSnap = Cast<UPrefabricatorConstructionSnapComponent>(Hit.GetComponent());
				if (CursorSnap && HitSnap) {
					FBox CursorBoxExtent = CursorSnap->GetScaledBoxExtent();
				}
			}
			else {
				CursorLocation = Hit.ImpactPoint;
				CursorRotation = FQuat::FindBetweenNormals(FVector(0, 0, 1), Hit.Normal);
				CursorRotation = CursorRotation * FQuat::MakeFromEuler(FVector(0, 0, CursorRotationDegrees));
			}
			FTransform CursorTransform;
			CursorTransform.SetLocation(CursorLocation);
			CursorTransform.SetRotation(CursorRotation);
			Cursor->SetTransform(CursorTransform);

			// Draw debug info
			DrawDebugPoint(World, Hit.ImpactPoint, 20, bHitSnapChannel ? FColor::Green : FColor::Red);
		}

		Cursor->SetVisiblity(bFoundHit);
	}
}

void UConstructionSystemBuildTool::RegisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::RegisterInputCallbacks(InputComponent);

	InputBindings.BuildAtCursor = InputComponent->BindAction("CSBuiltAtCursor", IE_Pressed, this, &UConstructionSystemBuildTool::ConstructAtCursor);
	InputBindings.CursorItemNext = InputComponent->BindAction("CSCursorItemNext", IE_Pressed, this, &UConstructionSystemBuildTool::CursorMoveNext);
	InputBindings.CursorItemPrev = InputComponent->BindAction("CSCursorItemPrev", IE_Pressed, this, &UConstructionSystemBuildTool::CursorMovePrev);

	InputBindings.CursorRotate = InputComponent->BindAxis("CSCursorRotate", this, &UConstructionSystemBuildTool::CursorRotate);
}

void UConstructionSystemBuildTool::UnregisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::UnregisterInputCallbacks(InputComponent);
	
	InputBindings.BuildAtCursor.ActionDelegate.Unbind();
	InputBindings.CursorItemNext.ActionDelegate.Unbind();
	InputBindings.CursorItemPrev.ActionDelegate.Unbind();
	InputBindings.CursorRotate.AxisDelegate.Unbind();

	InputBindings = FCSBuildToolInputBindings();
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

void UConstructionSystemBuildTool::CursorRotate(float RotationDelta)
{
	CursorRotationDegrees += CursorRotationStepAngle * RotationDelta;
}

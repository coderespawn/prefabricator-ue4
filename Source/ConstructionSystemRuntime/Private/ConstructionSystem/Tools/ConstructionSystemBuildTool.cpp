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


void UConstructionSystemBuildTool::InitializeTool(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::InitializeTool(ConstructionComponent);
	
	Cursor = NewObject<UConstructionSystemCursor>(this, "Cursor");
	Cursor->SetCursorMaterial(ConstructionComponent->CursorMaterial);
	Cursor->SetCursorInvalidMaterial(ConstructionComponent->CursorInvalidMaterial);

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
		Cursor->SetVisiblity(EConstructionSystemCursorVisiblity::Visible);
	}
}

void UConstructionSystemBuildTool::OnToolDisable(UConstructionSystemComponent* ConstructionComponent)
{
	UConstructionSystemTool::OnToolDisable(ConstructionComponent);

	if (Cursor) {
		Cursor->SetVisiblity(EConstructionSystemCursorVisiblity::Hidden);
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

		FVector CameraDirection = ViewRotation.RotateVector(FVector::ForwardVector);
		FVector StartLocation = ViewLocation + CameraDirection * FConstructionSystemConstants::BuildToolSweepRadius;
		FVector EndLocation = ViewLocation + CameraDirection * (TraceDistance + FConstructionSystemConstants::BuildToolSweepRadius);

		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(ConstructionComponent->GetOwner());
		if (Cursor->GetCursorGhostActor()) {
			FPrefabTools::IterateChildrenRecursive(Cursor->GetCursorGhostActor(), [&QueryParams](AActor* ChildCursorActor) {
				QueryParams.AddIgnoredActor(ChildCursorActor);
			});
		}

		FHitResult Hit;
		bCursorFoundHit = false;
		bCursorModeFreeForm = true;

		bool bHitSnapChannel = false;
		
		FCollisionShape SweepShape = FCollisionShape::MakeSphere(FConstructionSystemConstants::BuildToolSweepRadius);
		if (World->SweepSingleByChannel(Hit, StartLocation, EndLocation, FQuat::Identity, PrefabSnapChannel, SweepShape, QueryParams, ResponseParams)) {
			bCursorFoundHit = true;
			bHitSnapChannel = true;
		}
		else if (World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams, ResponseParams)) {
			// We did not hit anything. Trace in the static world
			bCursorFoundHit = true;
		}

		if (bCursorFoundHit) {
			FVector CursorLocation;
			FQuat CursorRotation;
			if (bHitSnapChannel) {
				// Snap the cursor
				UPrefabricatorConstructionSnapComponent* CursorSnap = Cursor->GetActiveSnapComponent();
				UPrefabricatorConstructionSnapComponent* HitSnap = Cast<UPrefabricatorConstructionSnapComponent>(Hit.GetComponent());
				if (CursorSnap && HitSnap) {
					FTransform TargetSnapTransform;
					if (FPCSnapUtils::GetSnapPoint(HitSnap, CursorSnap, Hit.ImpactPoint, TargetSnapTransform, CursorRotationStep, 100)) {
						bCursorModeFreeForm = false;
						CursorLocation = TargetSnapTransform.GetLocation();
						CursorRotation = TargetSnapTransform.GetRotation();
						DrawDebugPoint(World, CursorLocation, 20, FColor::Blue);
					}

				}
			}
			else {
				CursorLocation = Hit.ImpactPoint;
				CursorRotation = FQuat::FindBetweenNormals(FVector(0, 0, 1), Hit.Normal);
				float CursorRotationDegrees = CursorRotationStep * CursorRotationStepAngle;
				CursorRotation = CursorRotation * FQuat::MakeFromEuler(FVector(0, 0, CursorRotationDegrees));
			}
			FTransform CursorTransform;
			CursorTransform.SetLocation(CursorLocation);
			CursorTransform.SetRotation(CursorRotation);
			Cursor->SetTransform(CursorTransform);

			// Draw debug info
			DrawDebugPoint(World, Hit.ImpactPoint, 20, bHitSnapChannel ? FColor::Green : FColor::Red);
		}

		EConstructionSystemCursorVisiblity CursorVisiblity = EConstructionSystemCursorVisiblity::Visible;
		if (bCursorFoundHit) {

			UPrefabricatorConstructionSnapComponent* ActiveCursorSnap = Cursor->GetActiveSnapComponent();
			if (ActiveCursorSnap) {
				FVector BoxLocation = ActiveCursorSnap->GetComponentLocation();
				FRotator BoxRotation = ActiveCursorSnap->GetComponentRotation();
				FVector BoxExtent = ActiveCursorSnap->GetScaledBoxExtent();
				{
					FBox Box(-BoxExtent, BoxExtent);
					Box = Box.ExpandBy(-2);
					BoxExtent = Box.GetExtent();
				}

				TArray<FOverlapResult> Overlaps;
				if (World->OverlapMultiByChannel(Overlaps, BoxLocation, BoxRotation.Quaternion(), PrefabSnapChannel, FCollisionShape::MakeBox(BoxExtent), QueryParams)) {
					for (const FOverlapResult& Overlap : Overlaps) {
						if (UPrefabricatorConstructionSnapComponent* OverlapSnap = Cast<UPrefabricatorConstructionSnapComponent>(Overlap.GetComponent())) {
							if (ActiveCursorSnap->SnapType == EPrefabricatorConstructionSnapType::Wall && OverlapSnap->SnapType == EPrefabricatorConstructionSnapType::Wall) {
								FTransform TestTransform = OverlapSnap->GetComponentTransform();
								FTransform CursorTransform = ActiveCursorSnap->GetComponentTransform();
								FVector TestBounds = OverlapSnap->GetScaledBoxExtent();

								// Check if they are parallel
								bool bParallel = false;
								{
									FVector SideAxisA = TestTransform.GetUnitAxis((TestBounds.X > TestBounds.Y) ? EAxis::X : EAxis::Y);
									FVector SideAxisB = CursorTransform.GetUnitAxis((BoxExtent.X > BoxExtent.Y) ? EAxis::X : EAxis::Y);
									if (SideAxisA.Equals(SideAxisB)) {
										bParallel = true;
									}
								}

								if (!bParallel) {
									// Check if the walls are placed on the same plane
									FVector CursorZ = ActiveCursorSnap->GetComponentTransform().GetUnitAxis(EAxis::Z);
									FVector TestZ = OverlapSnap->GetComponentTransform().GetUnitAxis(EAxis::Z);
									if (CursorZ.Equals(TestZ)) {
										// The walls are on the same plane.  Check if the lines intersect
										FPlane TestPlane;
										{
											FVector LTestPlaneOrigin = FVector::ZeroVector;
											FVector LTestPlaneNormal = (TestBounds.X > TestBounds.Y) ? FVector(0, 1, 0) : FVector(1, 0, 0);

											FVector TestPlaneOrigin = TestTransform.TransformPosition(LTestPlaneOrigin);
											FVector TestPlaneNormal = TestTransform.TransformVector(LTestPlaneNormal);
											TestPlaneNormal.Normalize();
											TestPlane = FPlane(TestPlaneOrigin, TestPlaneNormal);
										}

										float CHalfSize = FMath::Max(BoxExtent.X, BoxExtent.Y);
										FVector RayOrigin = CursorTransform.TransformPosition(FVector::ZeroVector);
										FVector LRayDir = (BoxExtent.X > BoxExtent.Y) ? FVector(1, 0, 0) : FVector(0, 1, 0);
										FVector RayDir = CursorTransform.TransformVector(LRayDir);
										RayDir.Normalize();
										FVector PointOfIntersection = FMath::RayPlaneIntersection(RayOrigin, RayDir, TestPlane);
										float DistanceToIntersection = (RayOrigin - PointOfIntersection).Size();
										if (DistanceToIntersection >= CHalfSize - 1) {
											continue;
										}
									}
								}
							}

							CursorVisiblity = EConstructionSystemCursorVisiblity::VisibleInvalid;
							break;
						}
					}
				}
				//DrawDebugBox(World, BoxLocation, BoxExtent, BoxRotation.Quaternion(), FColor::Red);
			}
		}
		else {
			CursorVisiblity = EConstructionSystemCursorVisiblity::Hidden;
		}

		Cursor->SetVisiblity(CursorVisiblity);
	}
}

void UConstructionSystemBuildTool::RegisterInputCallbacks(UInputComponent* InputComponent)
{
	UConstructionSystemTool::RegisterInputCallbacks(InputComponent);

	InputBindings.BuildAtCursor = InputComponent->BindAction("CSBuiltAtCursor", IE_Pressed, this, &UConstructionSystemBuildTool::ConstructAtCursor);
	InputBindings.CursorItemNext = InputComponent->BindAction("CSCursorItemNext", IE_Pressed, this, &UConstructionSystemBuildTool::CursorMoveNext);
	InputBindings.CursorItemPrev = InputComponent->BindAction("CSCursorItemPrev", IE_Pressed, this, &UConstructionSystemBuildTool::CursorMovePrev);

	// TODO: Map bindings to cursor next/prev snap points

	InputBindings.CursorRotate = InputComponent->BindAxis("CSCursorRotate", this, &UConstructionSystemBuildTool::RotateCursorStep);
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

void UConstructionSystemBuildTool::RotateCursorStep(float NumSteps)
{
	CursorRotationStep += NumSteps;
}

void UConstructionSystemBuildTool::ServerConstructAtCursor_Implementation()
{
	ConstructAtCursor();

}

bool UConstructionSystemBuildTool::ServerConstructAtCursor_Validate()
{
	return true;
}

void UConstructionSystemBuildTool::ConstructAtCursor()
{
	UConstructionSystemComponent* ConstructionComponent = Cast<UConstructionSystemComponent>(GetOuter());
	if (!ConstructionComponent) {
		return;
	}

	if (bToolEnabled) {
		UWorld* World = ConstructionComponent->GetWorld();
		if (!World) {
			return;
		}

		/*
		if (ConstructionComponent->GetOwnerRole() != ROLE_Authority) {
			ServerConstructAtCursor();
			return;
		}
		*/
		if (Cursor->GetVisiblity() != EConstructionSystemCursorVisiblity::Visible) {
			// Current cursor location is invalid
			return;
		}
		if (World && ActivePrefabAsset) {
			FTransform Transform;
			if (Cursor->GetCursorTransform(Transform)) {
				APrefabActor* SpawnedPrefab = GetWorld()->SpawnActor<APrefabActor>(APrefabActor::StaticClass(), Transform);
				SpawnedPrefab->PrefabComponent->PrefabAssetInterface = ActivePrefabAsset;

				FRandomStream RandomStream(Cursor->GetCursorSeed());
				UPrefabricatorBlueprintLibrary::RandomizePrefab(SpawnedPrefab, RandomStream);

				if (!bCursorModeFreeForm) {
					// A prefab was created at the cursor on a snapped location. Reset the local cursor rotation
					CursorRotationStep = 0;
				}
			}
		}
	}
}

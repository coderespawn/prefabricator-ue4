//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/ConstructionSystemSnap.h"

#include "Components/ArrowComponent.h"
#include "Components/SphereComponent.h"
#include "PrimitiveSceneProxy.h"

///////////////////////////// UPrefabricatorBoxSnapComponent ///////////////////////////// 
UPrefabricatorConstructionSnapComponent::UPrefabricatorConstructionSnapComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoxExtent = FVector(100, 100, 100);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionProfileName("PrefabSnap");
}

void UPrefabricatorConstructionSnapComponent::OnRegister()
{
	Super::OnRegister();

}

FPrimitiveSceneProxy* UPrefabricatorConstructionSnapComponent::CreateSceneProxy()
{
	/** Represents a UBoxComponent to the scene manager. */
	class FBoxSceneProxy final : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FBoxSceneProxy(const UPrefabricatorConstructionSnapComponent* InComponent)
			: FPrimitiveSceneProxy(InComponent)
			, bDrawOnlyIfSelected(InComponent->bDrawOnlyIfSelected)
			, BoxExtents(InComponent->BoxExtent)
			, BoxColor(InComponent->ShapeColor)
			, LineThickness(InComponent->LineThickness)
			, SnapType(InComponent->SnapType)
			, FloorConstraint(InComponent->FloorConstraint)
			, WallConstraint(InComponent->WallConstraint)
		{
			bWillEverBeLit = false;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			QUICK_SCOPE_CYCLE_COUNTER(STAT_BoxSceneProxy_GetDynamicMeshElements);

			const FMatrix& LocalToWorld = GetLocalToWorld();

			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				if (VisibilityMap & (1 << ViewIndex))
				{
					const FSceneView* View = Views[ViewIndex];

					const FLinearColor DrawColor = GetViewSelectionColor(BoxColor, *View, IsSelected(), IsHovered(), false, IsIndividuallySelected());

					FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
					DrawOrientedWireBox(PDI, LocalToWorld.GetOrigin(), LocalToWorld.GetScaledAxis(EAxis::X), LocalToWorld.GetScaledAxis(EAxis::Y), LocalToWorld.GetScaledAxis(EAxis::Z), BoxExtents, DrawColor, SDPG_World, LineThickness);

					FVector AxisX = LocalToWorld.GetUnitAxis(EAxis::X);
					FVector AxisY = LocalToWorld.GetUnitAxis(EAxis::Y);
					FVector AxisZ = LocalToWorld.GetUnitAxis(EAxis::Z);
					
					{
						const float CircleRadius = 10.0f;
						const int32 NumCircleSegments = 16;

						if (IsSelected()) {
							bool bXP, bYP, bZP, bXN, bYN, bZN;
							bXP = bYP = bZP = bXN = bYN = bZN = false;

							if (SnapType == EPrefabricatorConstructionSnapType::Floor) {
								bXP = FloorConstraint.AttachX;
								bYP = FloorConstraint.AttachY;
								bZP = FloorConstraint.AttachZ;
								bXN = FloorConstraint.AttachXNegative;
								bYN = FloorConstraint.AttachYNegative;
								bZN = FloorConstraint.AttachZNegative;
							}
							else if (SnapType == EPrefabricatorConstructionSnapType::Wall) {
								FVector VX = LocalToWorld.TransformVector(FVector(BoxExtents.X, 0, 0));
								FVector VY = LocalToWorld.TransformVector(FVector(0, BoxExtents.Y, 0));
								bool bUseXAxis = VX.Size() > VY.Size();

								bZP = WallConstraint.AttachTop;
								bZN = WallConstraint.AttachBottom;

								if (bUseXAxis) {
									bXP = WallConstraint.AttachRight;
									bXN = WallConstraint.AttachLeft;
									bYP = false;
									bYN = false;
								}
								else {
									bYP = WallConstraint.AttachRight;
									bYN = WallConstraint.AttachLeft;
									bXP = false;
									bXN = false;
								}
							}

							// X+
							if (bXP) {
								FVector CX0 = LocalToWorld.TransformPosition(FVector(BoxExtents.X, 0, 0));
								DrawCircle(PDI, CX0, AxisY, AxisZ, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}

							// X-
							if (bXN) {
								FVector CX1 = LocalToWorld.TransformPosition(FVector(-BoxExtents.X, 0, 0));
								DrawCircle(PDI, CX1, AxisZ, AxisY, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}

							// Y+
							if (bYP) {
								FVector CY0 = LocalToWorld.TransformPosition(FVector(0, BoxExtents.Y, 0));
								DrawCircle(PDI, CY0, AxisZ, AxisX, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}
							
							// Y-
							if (bYN) {
								FVector CY1 = LocalToWorld.TransformPosition(FVector(0, -BoxExtents.Y, 0));
								DrawCircle(PDI, CY1, AxisX, AxisZ, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}

							// Z+
							if (bZP) {
								FVector CZ0 = LocalToWorld.TransformPosition(FVector(0, 0, BoxExtents.Z));
								DrawCircle(PDI, CZ0, AxisX, AxisY, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}

							// Z-
							if (bZN) {
								FVector CZ1 = LocalToWorld.TransformPosition(FVector(0, 0, -BoxExtents.Z));
								DrawCircle(PDI, CZ1, AxisY, AxisX, FColor::Red, CircleRadius, NumCircleSegments, SDPG_Foreground);
							}
						}
					}
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			const bool bProxyVisible = !bDrawOnlyIfSelected || IsSelected();

			// Should we draw this because collision drawing is enabled, and we have collision
			const bool bShowForCollision = View->Family->EngineShowFlags.Collision && IsCollisionEnabled();

			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = (IsShown(View) && bProxyVisible) || bShowForCollision;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }

	private:
		const uint32	bDrawOnlyIfSelected : 1;
		const FVector	BoxExtents;
		const FColor	BoxColor;
		const float LineThickness;
		EPrefabricatorConstructionSnapType SnapType;
		FPCSnapConstraintFloor FloorConstraint;
		FPCSnapConstraintWall WallConstraint;
	};

	return new FBoxSceneProxy(this);
}

///////////////////////////// AConstructionSnapPoint ///////////////////////////// 
APrefabricatorConstructionSnap::APrefabricatorConstructionSnap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	SetCanBeDamaged(false);
	bRelevantForLevelBounds = false;

	ConstructionSnapComponent = CreateDefaultSubobject<UPrefabricatorConstructionSnapComponent>(TEXT("SnapComponent"));
	ConstructionSnapComponent->Mobility = EComponentMobility::Static;
	ConstructionSnapComponent->SetGenerateOverlapEvents(true);
	RootComponent = ConstructionSnapComponent;

}


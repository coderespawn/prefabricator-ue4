//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemUtils.h"
#include "Engine/CollisionProfile.h"
#include "ConstructionSystemDefs.h"
#include "ConstructionSystemSnap.h"
#include "PrefabActor.h"

ECollisionChannel FConstructionSystemUtils::FindPrefabSnapChannel()
{
	// UCollisionProfile::ReturnContainerIndexFromChannelName is not exported with ENGINE_API so a less optimized way is used below
	UCollisionProfile* CollisionSettings = UCollisionProfile::Get();
	UEnum* Enum = StaticEnum<ECollisionChannel>();
	int32 NumEnums = Enum->NumEnums();
	for (int i = 0; i < NumEnums; i++) {
		FName ChannelName = CollisionSettings->ReturnChannelNameFromContainerIndex(i);
		if (ChannelName == FConstructionSystemConstants::PrefabSnapChannelName) {
			return (ECollisionChannel)Enum->GetValueByIndex(i);
		}
	}
	return ECollisionChannel::ECC_WorldStatic;
}

bool FPCSnapUtils::GetSnapPoint(UPrefabricatorConstructionSnapComponent* Src, UPrefabricatorConstructionSnapComponent* Dst, 
		const FVector& InRequestedSnapLocation, FTransform& OutTargetSnapTransform, float InSnapTolerrance)
{
	FTransform SrcWorldTransform = Src->GetComponentTransform();
	FTransform SrcWorldToLocal = SrcWorldTransform.Inverse();
	FVector LocalCursorSnapPosition = SrcWorldToLocal.TransformPosition(InRequestedSnapLocation);
	// Apply rotation to the local cursor scale
	{
		FVector SrcRelativeScale = Src->GetRelativeTransform().GetScale3D();
		SrcRelativeScale = SrcWorldTransform.GetRotation().RotateVector(SrcRelativeScale);
		LocalCursorSnapPosition *= SrcRelativeScale;
	}

	FTransform DstWorldTransform = Dst->GetComponentTransform();
	{
		AActor* Owner = Dst->GetOwner();
		APrefabActor* TopmostPrefab = nullptr;
		while (APrefabActor* PrefabActor = Cast<APrefabActor>(Owner->GetAttachParentActor())) {
			TopmostPrefab = PrefabActor;
			Owner = PrefabActor;
		}
		if (TopmostPrefab) {
			DstWorldTransform = DstWorldTransform * TopmostPrefab->GetActorTransform().Inverse();
		}
	}

	FVector SrcBoxExtent = Src->GetScaledBoxExtent();
	FVector DstBoxExtent = Dst->GetScaledBoxExtent();

	if (Src->SnapType == EPrefabricatorConstructionSnapType::Wall && Dst->SnapType == EPrefabricatorConstructionSnapType::Wall) {
		bool bUseSrcXAxis = SrcBoxExtent.X > SrcBoxExtent.Y;
		FVector2D SrcHalfSize2D;
		SrcHalfSize2D.X = bUseSrcXAxis ? SrcBoxExtent.X : SrcBoxExtent.Y;
		SrcHalfSize2D.Y = SrcBoxExtent.Z;

		bool bUseDstXAxis = DstBoxExtent.X > DstBoxExtent.Y;
		FVector2D DstHalfSize2D;
		DstHalfSize2D.X = bUseDstXAxis ? DstBoxExtent.X : DstBoxExtent.Y;
		DstHalfSize2D.Y = DstBoxExtent.Z;

		FVector2D Cursor2D;
		Cursor2D.X = bUseSrcXAxis ? LocalCursorSnapPosition.X : LocalCursorSnapPosition.Y;
		Cursor2D.Y = LocalCursorSnapPosition.Z;

		// Top
		FVector2D BestSrcPos2D = FVector2D(0, SrcHalfSize2D.Y);
		float BestSrcSnapDistance = FMath::Abs(Cursor2D.Y - SrcHalfSize2D.Y);

		FVector2D BestDstPos2D = FVector2D(0, -DstHalfSize2D.Y);

		// Bottom
		float TestDistance = FMath::Abs(Cursor2D.Y + SrcHalfSize2D.Y);
		if (TestDistance < BestSrcSnapDistance) {
			BestSrcSnapDistance = TestDistance;
			BestSrcPos2D = FVector2D(0, -SrcHalfSize2D.Y);
			BestDstPos2D = FVector2D(0, DstHalfSize2D.Y);
		}

		// Right
		TestDistance = FMath::Abs(Cursor2D.X - SrcHalfSize2D.X);
		if (TestDistance < BestSrcSnapDistance) {
			BestSrcSnapDistance = TestDistance;
			BestSrcPos2D = FVector2D(SrcHalfSize2D.X, 0);
			BestDstPos2D = FVector2D(-DstHalfSize2D.X, 0);
		}

		// Left
		TestDistance = FMath::Abs(Cursor2D.X + SrcHalfSize2D.X);
		if (TestDistance < BestSrcSnapDistance) {
			BestSrcSnapDistance = TestDistance;
			BestSrcPos2D = FVector2D(-SrcHalfSize2D.X, 0);
			BestDstPos2D = FVector2D(DstHalfSize2D.X, 0);
		}

		FVector BestLocalSrcSnapLocation;
		BestLocalSrcSnapLocation.Z = BestSrcPos2D.Y;
		BestLocalSrcSnapLocation.X = bUseSrcXAxis ? BestSrcPos2D.X : 0;
		BestLocalSrcSnapLocation.Y = bUseSrcXAxis ? 0 : BestSrcPos2D.X;

		FVector BestLocalDstSnapLocation;
		BestLocalDstSnapLocation.Z = BestDstPos2D.Y;
		BestLocalDstSnapLocation.X = bUseDstXAxis ? BestDstPos2D.X : 0;
		BestLocalDstSnapLocation.Y = bUseDstXAxis ? 0 : BestDstPos2D.X;

		BestLocalSrcSnapLocation /= Src->GetRelativeTransform().GetScale3D();
		BestLocalDstSnapLocation /= Dst->GetRelativeTransform().GetScale3D();

		FVector TargetSrcSnapLocation = SrcWorldTransform.TransformPosition(BestLocalSrcSnapLocation);
		FVector TargetDstSnapLocation = DstWorldTransform.TransformPosition(BestLocalDstSnapLocation);
		FRotator DstRotation = Src->GetComponentRotation();
		TargetDstSnapLocation = DstRotation.RotateVector(TargetDstSnapLocation);

		FVector DstOffset = TargetSrcSnapLocation - TargetDstSnapLocation;
		OutTargetSnapTransform = FTransform(DstRotation, DstOffset);
		return true;
	}

	return false;
}

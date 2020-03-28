//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/Thumbnail/PrefabricatorAssetThumbnailScene.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "PreviewScene.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabAssetThumbScene, Log, All);

FPrefabricatorAssetThumbnailScene::FPrefabricatorAssetThumbnailScene()
{
	bForceAllUsedMipsResident = false;

	// Create preview actor
	// checked
	FActorSpawnParameters SpawnInfo;
	SpawnInfo.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnInfo.bNoFail = true;
	SpawnInfo.ObjectFlags = RF_Transient;
	PreviewActor = GetWorld()->SpawnActor<APrefabActor>(SpawnInfo);

	PreviewActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
	PreviewActor->SetActorEnableCollision(false);

	Touch();
}

void FPrefabricatorAssetThumbnailScene::SetPrefabAsset(class UPrefabricatorAsset* PrefabAsset)
{
	PreviewActor->PrefabComponent->PrefabAssetInterface = PrefabAsset;
	if (PrefabAsset) {
		if (PreviewActor->IsPrefabOutdated()) {
			PreviewActor->LoadPrefab();
			FBox BoundsBox = FPrefabTools::GetPrefabBounds(PreviewActor);
			Bounds = FBoxSphereBounds(BoundsBox);
			UE_LOG(LogPrefabAssetThumbScene, Log, TEXT("Recreating Thumbnail Prefab Actor"));
		}

		PreviewActor->SetActorLocation(FVector(0, 0, 0), false);

		// Center the mesh at the world origin then offset to put it on top of the plane
		const float BoundsZOffset = GetBoundsZOffset(Bounds);
		PreviewActor->SetActorLocation(-Bounds.Origin + FVector(0, 0, BoundsZOffset), false);
	}
}

void FPrefabricatorAssetThumbnailScene::Touch()
{
	LastAccessTime = FDateTime::UtcNow();
}

void FPrefabricatorAssetThumbnailScene::GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const
{
	check(PreviewActor);
	check(PreviewActor->PrefabComponent);

	const float HalfFOVRadians = FMath::DegreesToRadians<float>(InFOVDegrees) * 0.5f;
	// Add extra size to view slightly outside of the sphere to compensate for perspective
	const float HalfMeshSize = Bounds.SphereRadius * 1.15;
	const float BoundsZOffset = GetBoundsZOffset(Bounds);
	const float TargetDistance = HalfMeshSize / FMath::Tan(HalfFOVRadians);

	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(PreviewActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous());

	if (PrefabAsset) {
		USceneThumbnailInfo* ThumbnailInfo = Cast<USceneThumbnailInfo>(PrefabAsset->ThumbnailInfo);
		if (ThumbnailInfo)
		{
			if (TargetDistance + ThumbnailInfo->OrbitZoom < 0)
			{
				ThumbnailInfo->OrbitZoom = -TargetDistance;
			}
		}
		else
		{
			ThumbnailInfo = USceneThumbnailInfo::StaticClass()->GetDefaultObject<USceneThumbnailInfo>();
		}

		OutOrigin = FVector(0, 0, -BoundsZOffset);
		OutOrbitPitch = ThumbnailInfo->OrbitPitch;
		OutOrbitYaw = ThumbnailInfo->OrbitYaw;
		OutOrbitZoom = TargetDistance + ThumbnailInfo->OrbitZoom;
	}


}


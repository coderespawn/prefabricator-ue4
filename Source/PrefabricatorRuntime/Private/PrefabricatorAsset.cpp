//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabricatorAsset.h"
#include "GameFramework/Actor.h"


UPrefabricatorAsset::UPrefabricatorAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

}


FVector FPrefabricatorAssetUtils::FindPivot(const TArray<AActor*>& InActors)
{
	FVector Pivot = FVector::ZeroVector;
	if (InActors.Num() > 0) {
		float LowestZ = MAX_flt;
		FVector BoundsOrigin, BoundsExtent;
		for (AActor* Actor : InActors) {
			Actor->GetActorBounds(false, BoundsOrigin, BoundsExtent);
			FVector Min = BoundsOrigin - BoundsExtent;
			LowestZ = FMath::Min(LowestZ, Min.Z);

			Pivot += BoundsOrigin;
		}
		Pivot /= InActors.Num();
		Pivot.Z = LowestZ;
	}
	return Pivot;
}

EComponentMobility::Type FPrefabricatorAssetUtils::FindMobility(const TArray<AActor*>& InActors)
{
	EComponentMobility::Type Mobility = EComponentMobility::Static;

	for (AActor* Actor : InActors) {
		if (!Actor || !Actor->GetRootComponent()) {
			continue;
		}
		EComponentMobility::Type ActorMobility = Actor->GetRootComponent()->Mobility;
		if (Mobility == EComponentMobility::Static && ActorMobility != EComponentMobility::Static) {
			Mobility = ActorMobility;
		}
		if (Mobility == EComponentMobility::Stationary && ActorMobility == EComponentMobility::Movable) {
			Mobility = EComponentMobility::Movable;
		}
	}

	return Mobility;
}

void FPrefabricatorAssetUtils::SaveActorData(AActor* InActor, const FVector& InPrefabPivot, FPrefabricatorActorData& OutActorData)
{

}


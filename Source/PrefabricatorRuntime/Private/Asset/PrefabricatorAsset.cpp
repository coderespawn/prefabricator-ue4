//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAsset.h"

#include "Prefab/PrefabTools.h"
#include "Utils/PrefabricatorService.h"

#include "GameFramework/Actor.h"

UPrefabricatorAsset::UPrefabricatorAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {

}

UPrefabricatorAsset* UPrefabricatorAsset::GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig)
{
	return this;
}

FVector FPrefabricatorAssetUtils::FindPivot(const TArray<AActor*>& InActors)
{
	FVector Pivot = FVector::ZeroVector;
	if (InActors.Num() > 0) {
		float LowestZ = MAX_flt;
		FBox Bounds(EForceInit::ForceInit);
		for (AActor* Actor : InActors) {
			FBox ActorBounds = FPrefabTools::GetPrefabBounds(Actor);
			Bounds += ActorBounds;
		}
		Pivot = Bounds.GetCenter();
		Pivot.Z = Bounds.Min.Z;
	}

	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Pivot = Service->SnapToGrid(Pivot);
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

UPrefabricatorAsset* UPrefabricatorAssetCollection::GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig)
{
	if (Prefabs.Num() == 0) return nullptr;

	float TotalWeight = 0.0f;
	for (const FPrefabricatorAssetCollectionItem& Item : Prefabs) {
		TotalWeight += FMath::Max(0.0f, Item.Weight);
	}

	FRandomStream Random;
	Random.Initialize(InConfig.Seed);

	if (TotalWeight == 0) {
		// Return a random value from the list
		int32 Index = Random.RandRange(0, Prefabs.Num() - 1);
		return Prefabs[Index].PrefabAsset;
	}

	float SelectionValue = Random.FRandRange(0, TotalWeight);
	float StartRange = 0.0f;
	for (const FPrefabricatorAssetCollectionItem& Item : Prefabs) {
		float EndRange = StartRange + Item.Weight;
		if (SelectionValue >= StartRange && SelectionValue < EndRange) {
			return Item.PrefabAsset;
		}
		StartRange = EndRange;
	}

	return Prefabs.Last().PrefabAsset;
}

void UPrefabricatorEventListener::PostSpawn_Implementation(APrefabActor* Prefab)
{

}


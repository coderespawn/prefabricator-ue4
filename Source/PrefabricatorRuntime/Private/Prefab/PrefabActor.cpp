//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/PrefabActor.h"

#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "Components/BillboardComponent.h"
#include "Engine/PointLight.h"
#include "PrefabricatorAssetUserData.h"
#include "PrefabricatorAsset.h"

APrefabActor::APrefabActor(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PrefabComponent = ObjectInitializer.CreateDefaultSubobject<UPrefabComponent>(this, "PrefabComponent");
	RootComponent = PrefabComponent;
}

namespace {
	void DestroyAttachedActorsRecursive(AActor* ActorToDestroy, TSet<AActor*>& Visited) {
		if (!ActorToDestroy || !ActorToDestroy->GetRootComponent()) return;

		if (Visited.Contains(ActorToDestroy)) return;
		Visited.Add(ActorToDestroy);

		UPrefabricatorAssetUserData* PrefabUserData = ActorToDestroy->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
		if (!PrefabUserData) return;

		UWorld* World = ActorToDestroy->GetWorld();
		if (!World) return;

		TArray<AActor*> AttachedActors;
		ActorToDestroy->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors) {
			DestroyAttachedActorsRecursive(AttachedActor, Visited);
		}
		ActorToDestroy->Destroy();
	}
}

void APrefabActor::Destroyed()
{
	Super::Destroyed();

	// Destroy all attached actors
	{
		TSet<AActor*> Visited;
		TArray<AActor*> AttachedActors;
		GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors) {
			DestroyAttachedActorsRecursive(AttachedActor, Visited);
		}
	}
}

void APrefabActor::PostLoad()
{
	Super::PostLoad();

}

void APrefabActor::PostActorCreated()
{
	Super::PostActorCreated();

}

#if WITH_EDITOR
void APrefabActor::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);


}

void APrefabActor::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	LoadPrefab();
}

FName APrefabActor::GetCustomIconName() const
{
	static const FName PrefabIconName("ClassIcon.PrefabActor");
	return PrefabIconName;
}

#endif // WITH_EDITOR

void APrefabActor::LoadPrefab()
{
	FPrefabTools::LoadStateFromPrefabAsset(this);
}

void APrefabActor::SavePrefab()
{
	FPrefabTools::SaveStateToPrefabAsset(this);
}

bool APrefabActor::IsPrefabOutdated()
{
	UPrefabricatorAsset* PrefabAsset = GetPrefabAsset();
	if (!PrefabAsset) {
		return false;
	}

	return PrefabAsset->LastUpdateID != LastUpdateID;
}

UPrefabricatorAsset* APrefabActor::GetPrefabAsset()
{
	FPrefabAssetSelectionConfig SelectionConfig;
	SelectionConfig.Seed = Seed;
	return PrefabComponent->PrefabAssetInterface ? PrefabComponent->PrefabAssetInterface->GetPrefabAsset(SelectionConfig) : nullptr;
}

void APrefabActor::RandomizeSeed()
{
	Seed = FMath::Rand();
}

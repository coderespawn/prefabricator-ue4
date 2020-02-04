//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorFunctionLibrary.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "Engine/Engine.h"

APrefabActor* UPrefabricatorBlueprintLibrary::SpawnPrefab(const UObject* WorldContextObject, UPrefabricatorAssetInterface* Prefab, const FTransform& Transform, int32 Seed)
{
	APrefabActor* PrefabActor = nullptr;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (World) {
		if (Prefab->bReplicates) {
			FActorSpawnParameters SpawnParams;
			PrefabActor = World->SpawnActor<AReplicablePrefabActor>(AReplicablePrefabActor::StaticClass(), Transform);
		}
		else {
			PrefabActor = World->SpawnActor<APrefabActor>(APrefabActor::StaticClass(), Transform);
		}
		

		if (PrefabActor) {
			PrefabActor->PrefabComponent->PrefabAssetInterface = Prefab;

			FRandomStream Random(Seed);
			RandomizePrefab(PrefabActor, Random);
		}
	}
	return PrefabActor;
}

void UPrefabricatorBlueprintLibrary::RandomizePrefab(APrefabActor* PrefabActor, const FRandomStream& InRandom)
{
	PrefabActor->RandomizeSeed(InRandom);

	FPrefabLoadSettings LoadSettings;
	LoadSettings.bRandomizeNestedSeed = true;
	LoadSettings.Random = &InRandom;
	FPrefabLoadStatePtr LoadState = MakeShareable(new FPrefabLoadState);
	FPrefabTools::LoadStateFromPrefabAsset(PrefabActor, LoadSettings, LoadState);
}

void UPrefabricatorBlueprintLibrary::GetAllAttachedActors(AActor* Prefab, TArray<AActor*>& AttachedActors)
{
	if (!Prefab) return;

	TArray<AActor*> ChildActors;
	Prefab->GetAttachedActors(ChildActors);
	for (AActor* ChildActor : ChildActors) {
		AttachedActors.Add(ChildActor);
		GetAllAttachedActors(ChildActor, AttachedActors);
	}
}


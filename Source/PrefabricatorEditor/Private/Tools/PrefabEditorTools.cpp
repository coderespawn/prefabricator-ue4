//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Tools/PrefabEditorTools.h"

#include "PrefabActor.h"
#include "PrefabricatorAsset.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "PrefabricatorAssetUserData.h"
#include "GameFramework/Actor.h"

bool FPrefabEditorTools::CanCreatePrefab()
{
	if (!GEditor) {
		return false;
	}

	int32 NumSelected = GEditor->GetSelectedActorCount();
	return NumSelected > 0;
}

void FPrefabEditorTools::CreatePrefab()
{
	TArray<AActor*> SelectedActors;
	GetSelectedActors(SelectedActors);

	CreatePrefabFromActors(SelectedActors);
}

void FPrefabEditorTools::CreatePrefabFromActors(const TArray<AActor*>& Actors)
{
	if (Actors.Num() == 0) {
		return;
	}

	UWorld* World = Actors[0]->GetWorld();

	FVector Pivot = FPrefabricatorAssetUtils::FindPivot(Actors);
	APrefabActor* PrefabActor = World->SpawnActor<APrefabActor>(Pivot, FRotator::ZeroRotator);

	// Find the compatible mobility for the prefab actor
	EComponentMobility::Type Mobility = FPrefabricatorAssetUtils::FindMobility(Actors);
	PrefabActor->GetRootComponent()->SetMobility(Mobility);

	// Attach the actors to the prefab
	for (AActor* Actor : Actors) {
		if (Actor->GetRootComponent()) {
			Actor->GetRootComponent()->SetMobility(Mobility);
			AddAssetUserData(Actor, PrefabActor);
		}
		FAttachmentTransformRules AttachmentRules(EAttachmentRule::KeepWorld, true);
		Actor->AttachToActor(PrefabActor, AttachmentRules);
	}

	if (GEditor) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(PrefabActor, true, true);
	}
}

void FPrefabEditorTools::GetSelectedActors(TArray<AActor*>& OutActors)
{
	if (GEditor) {
		USelection* SelectedActors = GEditor->GetSelectedActors();
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			// We only care about actors that are referenced in the world for literals, and also in the same level as this blueprint
			AActor* Actor = Cast<AActor>(*Iter);
			if (Actor) 
			{
				OutActors.Add(Actor);
			}
		}
	}
}

void FPrefabEditorTools::AddAssetUserData(AActor* InActor, APrefabActor* Prefab)
{
	if (!InActor || !InActor->GetRootComponent()) {
		return;
	}
	
	UPrefabricatorAssetUserData* PrefabUserData = NewObject<UPrefabricatorAssetUserData>(InActor->GetRootComponent());
	PrefabUserData->PrefabActor = Prefab;
	InActor->GetRootComponent()->AddAssetUserData(PrefabUserData);
}


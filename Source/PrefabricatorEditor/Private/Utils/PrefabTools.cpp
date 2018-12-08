//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabTools.h"
#include "Engine/Selection.h"
#include "Editor/EditorEngine.h"

bool FPrefabTools::CanCreatePrefab()
{
	if (!GEditor) {
		return false;
	}

	int32 NumSelected = GEditor->GetSelectedActorCount();
	return NumSelected > 0;
}

void FPrefabTools::CreatePrefab()
{
	TArray<AActor*> SelectedActors;
	GetSelectedActors(SelectedActors);

	CreatePrefabFromActors(SelectedActors);
}

void FPrefabTools::CreatePrefabFromActors(const TArray<AActor*>& Actors)
{
	

}

void FPrefabTools::GetSelectedActors(TArray<AActor*>& OutActors)
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

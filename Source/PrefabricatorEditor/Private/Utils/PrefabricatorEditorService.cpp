//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorEditorService.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"

void FPrefabricatorEditorService::ParentActors(AActor* ParentActor, AActor* ChildActor)
{
	if (GEditor) {
		GEditor->ParentActors(ParentActor, ChildActor, NAME_None);
	}
}

void FPrefabricatorEditorService::SelectPrefabActor(AActor* PrefabActor)
{
	if (GEditor) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(PrefabActor, true, true);
	}
}

void FPrefabricatorEditorService::GetSelectedActors(TArray<AActor*>& OutActors)
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

int FPrefabricatorEditorService::GetNumSelectedActors()
{
	return GEditor ? GEditor->GetSelectedActorCount() : 0;
}


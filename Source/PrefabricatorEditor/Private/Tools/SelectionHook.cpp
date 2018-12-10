//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Tools/SelectionHook.h"

#include "PrefabActor.h"
#include "PrefabricatorAssetUserData.h"

#include "Engine/Selection.h"
#include "GameFramework/Actor.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorSelectionHook, Log, All);

void FPrefabricatorSelectionHook::Initialize()
{
	USelection::SelectObjectEvent.AddRaw(this, &FPrefabricatorSelectionHook::OnObjectSelected);
}

void FPrefabricatorSelectionHook::Release()
{
	USelection::SelectObjectEvent.Remove(CallbackHandle);
}

void FPrefabricatorSelectionHook::OnObjectSelected(UObject* Object)
{
	//UE_LOG(LogPrefabricatorSelectionHook, Log, TEXT("Object Selection Changed: %s"), (Object ? *Object->GetName() : TEXT("None")));

	if (Object->IsA<APrefabActor>()) {

	}
	else {
		AActor* Actor = Cast<AActor>(Object);
		if (Actor && Actor->GetRootComponent()) {
			UPrefabricatorAssetUserData* PrefabUserData = Actor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
			if (PrefabUserData && PrefabUserData->PrefabActor.IsValid()) {
				GEditor->SelectActor(Actor, false, true);
				GEditor->SelectActor(PrefabUserData->PrefabActor.Get(), true, true);
			}
		}
	}
}


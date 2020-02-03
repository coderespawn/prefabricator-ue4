//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabEditorTools.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"

#include "EditorViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"

void FPrefabEditorTools::ReloadPrefabsInLevel(UWorld* World, UPrefabricatorAsset* InAsset)
{
	for (TActorIterator<APrefabActor> It(World); It; ++It) {
		APrefabActor* PrefabActor = *It;
		if (PrefabActor && PrefabActor->PrefabComponent) {
			bool bShouldRefresh = true;
			// The provided asset can be null, in which case we refresh everything, else we search if it matches the particular prefab asset
			if (InAsset) {
				UPrefabricatorAsset* ActorAsset = PrefabActor->GetPrefabAsset();
				bShouldRefresh = (InAsset == ActorAsset);
			}
			if (bShouldRefresh) {
				if (PrefabActor->IsPrefabOutdated()) {
					PrefabActor->LoadPrefab();
				}
			}
		}
	}
}

void FPrefabEditorTools::ShowNotification(FText Text, SNotificationItem::ECompletionState State /*= SNotificationItem::CS_Fail*/)
{
	FNotificationInfo Info(Text);
	Info.bFireAndForget = true;
	Info.FadeOutDuration = 1.0f;
	Info.ExpireDuration = 2.0f;

	TWeakPtr<SNotificationItem> NotificationPtr = FSlateNotificationManager::Get().AddNotification(Info);
	if (NotificationPtr.IsValid())
	{
		NotificationPtr.Pin()->SetCompletionState(State);
	}
}

void FPrefabEditorTools::SwitchLevelViewportToRealtimeMode()
{
	FEditorViewportClient* Client = (FEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
	if (Client) {
		bool bRealtime = Client->IsRealtime();
		if (!bRealtime) {
			ShowNotification(NSLOCTEXT("Prefabricator", "PrefabRealtimeMode", "Switched viewport to Realtime mode"), SNotificationItem::CS_None);
			Client->SetRealtime(true);
		}
	}
	else {
		ShowNotification(NSLOCTEXT("Prefabricator", "PRefabClientNotFound", "Warning: Cannot find active viewport"));
	}
}


//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabEditorTools.h"

#include "Asset/Thumbnail/PrefabricatorAssetThumbnailScene.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"

#include "EditorViewportClient.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineModule.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "LegacyScreenPercentageDriver.h"

void FPrefabEditorTools::UpdateAssetThumbnail_GameThread(UPrefabricatorAsset* InAsset)
{
	check(IsInGameThread());

	const int32 DefaultThumbnailSize = 256;
	UTextureRenderTarget2D* RenderTexture = NewObject<UTextureRenderTarget2D>();
	check(RenderTexture);
	RenderTexture->RenderTargetFormat = RTF_RG8;
	RenderTexture->ClearColor = FLinearColor::Black;
	RenderTexture->bAutoGenerateMips = true;
	RenderTexture->InitAutoFormat(DefaultThumbnailSize, DefaultThumbnailSize);
	RenderTexture->UpdateResourceImmediate(true);

	FRenderTarget* RenderTarget = RenderTexture->GameThread_GetRenderTargetResource();

	//FRenderTarget* Render
	ENQUEUE_RENDER_COMMAND(UpdateThumbCommand)(
		[InAsset, RenderTarget](FRHICommandListImmediate& RHICmdList)
	{
		FPrefabEditorTools::UpdateAssetThumbnail_RenderThread(RHICmdList, InAsset, RenderTarget);
	});
}

void FPrefabEditorTools::UpdateAssetThumbnail_RenderThread(FRHICommandListImmediate& RHICmdList, UPrefabricatorAsset* InAsset, FRenderTarget* RenderTarget)
{
	check(IsInRenderingThread());

	FPrefabricatorAssetThumbnailScene ThumbnailScene;
	ThumbnailScene.SetPrefabAsset(InAsset);
	ThumbnailScene.GetScene()->UpdateSpeedTreeWind(0.0);


	FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene.GetScene(), FEngineShowFlags(ESFIM_Game))
		.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

	ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
	ViewFamily.EngineShowFlags.MotionBlur = 0;
	ViewFamily.EngineShowFlags.LOD = 0;

	ThumbnailScene.GetView(&ViewFamily, 0, 0, 256, 256);

	ViewFamily.EngineShowFlags.ScreenPercentage = false;
	ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
		ViewFamily, /* GlobalResolutionFraction = */ 1.0f, /* AllowPostProcessSettingsScreenPercentage = */ false));

	//GetRendererModule().BeginRenderingViewFamily(Canvas, &ViewFamily);

}

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


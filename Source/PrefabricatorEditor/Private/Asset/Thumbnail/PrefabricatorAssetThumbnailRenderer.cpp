//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/Thumbnail/PrefabricatorAssetThumbnailRenderer.h"
#include "PrefabricatorAssetThumbnailScene.h"
#include "PrefabricatorAsset.h"
#include "RenderingThread.h"
#include "SceneView.h"


UPrefabricatorAssetThumbnailRenderer::UPrefabricatorAssetThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ThumbnailScene = nullptr;
}

void UPrefabricatorAssetThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas)
{
	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(Object);
	if (PrefabAsset && !PrefabAsset->IsPendingKill()) {
		if (ThumbnailScene == nullptr || ensure(ThumbnailScene->GetWorld() != nullptr) == false) {
			if (ThumbnailScene) {
				FlushRenderingCommands();
				delete ThumbnailScene;
			}
			ThumbnailScene = new FPrefabricatorAssetThumbnailScene();
		}

		ThumbnailScene->SetPrefabAsset(PrefabAsset);
		ThumbnailScene->GetScene()->UpdateSpeedTreeWind(0.0);

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		ThumbnailScene->GetView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily);
		ThumbnailScene->SetPrefabAsset(nullptr);
	}
}

void UPrefabricatorAssetThumbnailRenderer::BeginDestroy()
{
	if (ThumbnailScene != nullptr)
	{
		delete ThumbnailScene;
		ThumbnailScene = nullptr;
	}

	Super::BeginDestroy();
}


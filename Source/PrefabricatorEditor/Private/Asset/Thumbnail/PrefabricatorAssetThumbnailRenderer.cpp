//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/Thumbnail/PrefabricatorAssetThumbnailRenderer.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/Thumbnail/PrefabricatorAssetThumbnailScene.h"

#include "RenderingThread.h"
#include "SceneView.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabAssetThumbRenderer, Log, All);


UPrefabricatorAssetThumbnailRenderer::UPrefabricatorAssetThumbnailRenderer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UPrefabricatorAssetThumbnailRenderer::Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas)
{
	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(Object);
	if (PrefabAsset && !PrefabAsset->IsPendingKill()) {
		FPrefabricatorAssetThumbnailScene* ThumbnailScene = GetThumbnailScene(*PrefabAsset->GetPathName());
		ThumbnailScene->SetPrefabAsset(PrefabAsset);
		ThumbnailScene->GetScene()->UpdateSpeedTreeWind(0.0);

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RenderTarget, ThumbnailScene->GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		ThumbnailScene->GetView(&ViewFamily, X, Y, Width, Height);
		RenderViewFamily(Canvas, &ViewFamily);
		
	}
}

void UPrefabricatorAssetThumbnailRenderer::BeginDestroy()
{
	for (auto& Entry : AssetThumbnailScenes) {
		FPrefabricatorAssetThumbnailScene* ThumbnailScene = Entry.Value;
		delete ThumbnailScene; 
		ThumbnailScene = nullptr;
	}
	AssetThumbnailScenes.Reset();

	Super::BeginDestroy();
}

FPrefabricatorAssetThumbnailScene* UPrefabricatorAssetThumbnailRenderer::GetThumbnailScene(const FName& InAssetPath)
{
	FPrefabricatorAssetThumbnailScene** SearchResult = AssetThumbnailScenes.Find(InAssetPath);
	FPrefabricatorAssetThumbnailScene* ThumbnailScene = SearchResult ? *SearchResult : nullptr;
	if (ThumbnailScene == nullptr || ensure(ThumbnailScene->GetWorld() != nullptr) == false) {
		if (ThumbnailScene) {
			FlushRenderingCommands();
			delete ThumbnailScene;
		}
		ThumbnailScene = new FPrefabricatorAssetThumbnailScene();
		AssetThumbnailScenes.Remove(InAssetPath);
		AssetThumbnailScenes.Add(InAssetPath, ThumbnailScene);
	}
	ThumbnailScene->Touch();
	return ThumbnailScene;
}

void UPrefabricatorAssetThumbnailRenderer::PurgeUnusedThumbnailScenes()
{
	FDateTime CurrentTime = FDateTime::UtcNow();

	const double MAX_IDLE_TIME_SECS = 5.0;	// in seconds

	TSet<FName> ItemsToPurge;
	for (auto& Entry : AssetThumbnailScenes) {
		FPrefabricatorAssetThumbnailScene* ThumbnailScene = Entry.Value;
		FTimespan ElapsedTime = CurrentTime - ThumbnailScene->LastAccessTime;
		float ElapsedSeconds = ElapsedTime.GetTotalSeconds();
		if (ElapsedTime >= MAX_IDLE_TIME_SECS) {
			ItemsToPurge.Add(Entry.Key);
		}
	}

	for (const FName& KeyToPurge : ItemsToPurge) {
		FPrefabricatorAssetThumbnailScene* ThumbnailScene = AssetThumbnailScenes[KeyToPurge];
		delete ThumbnailScene;
		ThumbnailScene = nullptr;

		AssetThumbnailScenes.Remove(KeyToPurge);
		UE_LOG(LogPrefabAssetThumbRenderer, Log, TEXT("Purged prefab asset thumbnail render scene: %s"), *KeyToPurge.ToString());
	}
}


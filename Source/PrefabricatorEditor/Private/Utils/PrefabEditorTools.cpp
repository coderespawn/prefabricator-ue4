//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabEditorTools.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"

#include "EditorViewportClient.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "ObjectTools.h"
#include "AssetData.h"
#include "Asset/PrefabricatorAsset.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Asset/Thumbnail/PrefabricatorAssetThumbnailScene.h"
#include "EngineModule.h"
#include "LegacyScreenPercentageDriver.h"
#include "Engine/Canvas.h"

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

void FPrefabEditorTools::CapturePrefabAssetThumbnail(UPrefabricatorAsset* InAsset)
{
	int32 ThumbSize = 512;

	TArray<FColor> Bitmap;
	{
		FPrefabricatorAssetThumbnailScene ThumbnailScene;
		UWorld* World = ThumbnailScene.GetWorld();
		ThumbnailScene.SetPrefabAsset(InAsset);

		UTextureRenderTarget2D* RTT = UKismetRenderingLibrary::CreateRenderTarget2D(World, ThumbSize, ThumbSize, RTF_RGBA32f);
		FTextureRenderTargetResource* RTTResource = RTT->GameThread_GetRenderTargetResource();

		FSceneViewFamilyContext ViewFamily(FSceneViewFamily::ConstructionValues(RTTResource, ThumbnailScene.GetScene(), FEngineShowFlags(ESFIM_Game))
			.SetWorldTimes(FApp::GetCurrentTime() - GStartTime, FApp::GetDeltaTime(), FApp::GetCurrentTime() - GStartTime));

		ViewFamily.EngineShowFlags.DisableAdvancedFeatures();
		ViewFamily.EngineShowFlags.MotionBlur = 0;
		ViewFamily.EngineShowFlags.LOD = 0;

		ThumbnailScene.GetView(&ViewFamily, 0, 0, ThumbSize, ThumbSize);

		ViewFamily.EngineShowFlags.ScreenPercentage = false;
		ViewFamily.SetScreenPercentageInterface(new FLegacyScreenPercentageDriver(
			ViewFamily, /* GlobalResolutionFraction = */ 1.0f, /* AllowPostProcessSettingsScreenPercentage = */ false));

		FVector2D CanvasSize;
		UCanvas* Canvas = nullptr;
		FDrawToRenderTargetContext RenderContext;
		UKismetRenderingLibrary::BeginDrawCanvasToRenderTarget(World, RTT, Canvas, CanvasSize, RenderContext);

		GetRendererModule().BeginRenderingViewFamily(Canvas->Canvas, &ViewFamily);
		Canvas->Canvas->Flush_GameThread(true);

		ENQUEUE_RENDER_COMMAND(UpdateThumbnailRTCommand)(
			[RTTResource](FRHICommandListImmediate& RHICmdList)
			{
				// Copy (resolve) the rendered thumbnail from the render target to its texture
				RHICmdList.CopyToResolveTarget(
					RTTResource->GetRenderTargetTexture(),		// Source texture
					RTTResource->TextureRHI,					// Dest texture
					FResolveParams() );									// Resolve parameters
			});

		
		RTTResource->ReadPixels(Bitmap);
		check(Bitmap.Num() == ThumbSize * ThumbSize);

		UKismetRenderingLibrary::EndDrawCanvasToRenderTarget(World, RenderContext);
		UKismetRenderingLibrary::ReleaseRenderTarget2D(RTT);
	}
	/*
	Bitmap.AddUninitialized(ThumbSize * ThumbSize);
	for (int i = 0; i < Bitmap.Num(); i++) {
		Bitmap[i] = FColor::Blue;
	}
	*/

	//setup actual thumbnail
	FObjectThumbnail TempThumbnail;
	TempThumbnail.SetImageSize(ThumbSize, ThumbSize);
	TArray<uint8>& ThumbnailByteArray = TempThumbnail.AccessImageData();

	// Copy scaled image into destination thumb
	int32 MemorySize = ThumbSize * ThumbSize * sizeof(FColor);
	ThumbnailByteArray.AddUninitialized(MemorySize);
	FMemory::Memcpy(&(ThumbnailByteArray[0]), &(Bitmap[0]), MemorySize);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");

	//check if each asset should receive the new thumb nail
	{
		const FAssetData CurrentAsset = FAssetData(InAsset);

		//assign the thumbnail and dirty
		const FString ObjectFullName = CurrentAsset.GetFullName();
		const FString PackageName = CurrentAsset.PackageName.ToString();

		UPackage* AssetPackage = FindObject<UPackage>(NULL, *PackageName);
		if (ensure(AssetPackage))
		{
			FObjectThumbnail* NewThumbnail = ThumbnailTools::CacheThumbnail(ObjectFullName, &TempThumbnail, AssetPackage);
			if (ensure(NewThumbnail))
			{
				//we need to indicate that the package needs to be re-saved
				AssetPackage->MarkPackageDirty();

				// Let the content browser know that we've changed the thumbnail
				NewThumbnail->MarkAsDirty();

				// Signal that the asset was changed if it is loaded so thumbnail pools will update
				if (CurrentAsset.IsAssetLoaded())
				{
					CurrentAsset.GetAsset()->PostEditChange();
				}

				//Set that thumbnail as a valid custom thumbnail so it'll be saved out
				NewThumbnail->SetCreatedAfterCustomThumbsEnabled();
			}
		}
	}
}


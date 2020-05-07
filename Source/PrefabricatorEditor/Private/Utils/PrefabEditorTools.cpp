//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabEditorTools.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/Thumbnail/PrefabricatorAssetThumbnailScene.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "PrefabricatorSettings.h"

#include "AssetData.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "EditorViewportClient.h"
#include "Engine/Canvas.h"
#include "Engine/TextureRenderTarget2D.h"
#include "EngineModule.h"
#include "EngineUtils.h"
#include "Framework/Notifications/NotificationManager.h"
#include "IContentBrowserSingleton.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "LegacyScreenPercentageDriver.h"
#include "Modules/ModuleManager.h"
#include "ObjectTools.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

namespace {
	template<typename T>
	static T* CreateAssetOnContentBrowser(const FString& InAssetName, bool bSyncBrowserToAsset)
	{
		IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
		TArray<FString> SelectedFolders;
		ContentBrowserSingleton.GetSelectedPathViewFolders(SelectedFolders);
		FString AssetFolder = SelectedFolders.Num() > 0 ? SelectedFolders[0] : "/Game";
		FString AssetPath = AssetFolder + "/" + InAssetName;

		FString PackageName, AssetName;
		IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
		AssetTools.CreateUniqueAssetName(*AssetPath, TEXT(""), PackageName, AssetName);
		T* AssetObject = Cast<T>(AssetTools.CreateAsset(AssetName, AssetFolder, T::StaticClass(), nullptr));
		if (AssetObject && bSyncBrowserToAsset) {
			ContentBrowserSingleton.SyncBrowserToAssets(TArray<UObject*>({ AssetObject }));
		}

		return AssetObject;
	}
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

	AssignPrefabAssetThumbnail(InAsset, Bitmap, ThumbSize, ThumbSize);
}

void FPrefabEditorTools::AssignPrefabAssetThumbnail(UPrefabricatorAssetInterface* InAsset, const TArray<FColor>& InBitmap, int32 Width, int32 Height)
{
	if (InBitmap.Num() == 0 || Width == 0 || Height == 0) return;
	check(InBitmap.Num() == Width * Height);

	//setup actual thumbnail
	FObjectThumbnail TempThumbnail;
	TempThumbnail.SetImageSize(Width, Height);
	TArray<uint8>& ThumbnailByteArray = TempThumbnail.AccessImageData();

	// Copy scaled image into destination thumb
	int32 MemorySize = Width * Height * sizeof(FColor);
	ThumbnailByteArray.AddUninitialized(MemorySize);
	FMemory::Memcpy(&(ThumbnailByteArray[0]), &(InBitmap[0]), MemorySize);

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

void FPrefabEditorTools::AssignPrefabAssetThumbnail(UPrefabricatorAssetInterface* InAsset, UTexture2D* ThumbTexture)
{
	TArray<FColor> Bitmap;
	if (!ThumbTexture) return;

	int32 TexWidth = ThumbTexture->GetSizeX();
	int32 TexHeight = ThumbTexture->GetSizeY();
	Bitmap.AddUninitialized(TexWidth * TexHeight);
	const FColor* FormatedImageData = static_cast<const FColor*>(ThumbTexture->PlatformData->Mips[0].BulkData.LockReadOnly());
	if (FormatedImageData) {
		for (int32 X = 0; X < TexWidth; X++) {
			for (int32 Y = 0; Y < TexHeight; Y++) {
				int32 Idx = Y * TexWidth + X;
				FColor PixelColor = FormatedImageData[Idx];
				FLinearColor LinearColor(PixelColor);
				LinearColor *= LinearColor.A;
				Bitmap[Idx] = LinearColor.ToFColor(false);
			}
		}
	}
	ThumbTexture->PlatformData->Mips[0].BulkData.Unlock();
	AssignPrefabAssetThumbnail(InAsset, Bitmap, TexWidth, TexHeight);

	/*
	const FColor* MipData = reinterpret_cast<FColor*>(ThumbTexture->PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY));
	if (MipData) {
		const int32 TexWidth = ThumbTexture->GetSurfaceWidth();
		const int32 TexHeight = ThumbTexture->GetSurfaceHeight();

		Bitmap.AddUninitialized(TexWidth * TexHeight);
		for (int32 Y = 0; Y < TexHeight; ++Y) {
			for (int32 X = 0; X < TexWidth; ++X) {
				int32 Idx = Y * TexWidth + X;
				Bitmap[Idx] = Y > TexHeight / 2 ? FColor::Red : FColor::Blue; // MipData[Idx];
			}
		}
		AssignPrefabAssetThumbnail(InAsset, Bitmap, TexWidth, TexHeight);
	}
	ThumbTexture->PlatformData->Mips[0].BulkData.Unlock();
	*/
}

UThumbnailInfo* FPrefabEditorTools::CreateDefaultThumbInfo(UPrefabricatorAsset* InAsset)
{
	// Thumb info doesn't exist. Create one
	USceneThumbnailInfo* SceneThumbnailInfo = NewObject<USceneThumbnailInfo>(InAsset, NAME_None, RF_Transactional);
	if (SceneThumbnailInfo) {
		// Grab the default values from the project settings
		const UPrefabricatorSettings* Settings = GetDefault<UPrefabricatorSettings>();
		if (Settings) {
			SceneThumbnailInfo->OrbitPitch = Settings->DefaultThumbnailPitch;
			SceneThumbnailInfo->OrbitYaw = Settings->DefaultThumbnailYaw;
			SceneThumbnailInfo->OrbitZoom = Settings->DefaultThumbnailZoom;
		}
	}
	return SceneThumbnailInfo;
}

UPrefabricatorAsset* FPrefabEditorTools::CreatePrefabAsset()
{
	UPrefabricatorAsset* PrefabAsset = CreateAssetOnContentBrowser<UPrefabricatorAsset>("Prefab", true);
	PrefabAsset->ThumbnailInfo = FPrefabEditorTools::CreateDefaultThumbInfo(PrefabAsset);
	return PrefabAsset;
}

UPrefabricatorAssetCollection* FPrefabEditorTools::CreatePrefabCollectionAsset()
{
	return CreateAssetOnContentBrowser<UPrefabricatorAssetCollection>("PrefabCollection", true);
}


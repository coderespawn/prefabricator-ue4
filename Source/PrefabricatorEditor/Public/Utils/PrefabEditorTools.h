//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Widgets/Notifications/SNotificationList.h"

class UPrefabricatorAsset;
class UPrefabricatorAssetInterface;
class UPrefabricatorAssetCollection;
class UThumbnailInfo;

class PREFABRICATOREDITOR_API FPrefabEditorTools {
public:
	static void ReloadPrefabsInLevel(UWorld* World, UPrefabricatorAsset* InAsset = nullptr);

	static void ShowNotification(FText Text, SNotificationItem::ECompletionState State = SNotificationItem::CS_Fail);

	static void SwitchLevelViewportToRealtimeMode();

	static void CapturePrefabAssetThumbnail(UPrefabricatorAsset* InAsset);
	static void AssignPrefabAssetThumbnail(UPrefabricatorAssetInterface* InAsset, const TArray<FColor>& InBitmap, int32 Width, int32 Height);
	static void AssignPrefabAssetThumbnail(UPrefabricatorAssetInterface* InAsset, UTexture2D* ThumbTexture);

	static UThumbnailInfo* CreateDefaultThumbInfo(UPrefabricatorAsset* InAsset);
	static UPrefabricatorAsset* CreatePrefabAsset();
	static UPrefabricatorAssetCollection* CreatePrefabCollectionAsset();

};


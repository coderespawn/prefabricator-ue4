//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IAssetTools.h"
#include "IContentBrowserSingleton.h"
#include "Widgets/Notifications/SNotificationList.h"

class UPrefabricatorAsset;

class PREFABRICATOREDITOR_API FPrefabEditorTools {
public:
	static void ReloadPrefabsInLevel(UWorld* World, UPrefabricatorAsset* InAsset = nullptr);

	static void ShowNotification(FText Text, SNotificationItem::ECompletionState State = SNotificationItem::CS_Fail);

	static void SwitchLevelViewportToRealtimeMode();

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

};


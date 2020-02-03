//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ThumbnailRendering/DefaultSizedThumbnailRenderer.h"
#include "UObject/ObjectMacros.h"
#include "PrefabricatorAssetThumbnailRenderer.generated.h"

class FCanvas;
class FRenderTarget;
class FPrefabricatorAssetThumbnailScene;

UCLASS()
class PREFABRICATOREDITOR_API UPrefabricatorAssetThumbnailRenderer : public UDefaultSizedThumbnailRenderer
{
	GENERATED_UCLASS_BODY()


	// Begin UThumbnailRenderer Object
	virtual void Draw(UObject* Object, int32 X, int32 Y, uint32 Width, uint32 Height, FRenderTarget* RenderTarget, FCanvas* Canvas) override;
	// End UThumbnailRenderer Object

	// UObject implementation
	virtual void BeginDestroy() override;

private:
	FPrefabricatorAssetThumbnailScene* GetThumbnailScene(const FName& InAssetPath);
	void PurgeUnusedThumbnailScenes();

private:
	TMap<FName, FPrefabricatorAssetThumbnailScene*> AssetThumbnailScenes;
};


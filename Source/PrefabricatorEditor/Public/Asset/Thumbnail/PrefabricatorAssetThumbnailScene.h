//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ThumbnailHelpers.h"

class PREFABRICATOREDITOR_API FPrefabricatorAssetThumbnailScene : public FThumbnailPreviewScene
{
public:
	/** Constructor */
	FPrefabricatorAssetThumbnailScene();

	/** Sets the static mesh to use in the next GetView() */
	void SetPrefabAsset(class UPrefabricatorAsset* PrefabAsset);

	void Touch();
	FDateTime LastAccessTime;

protected:
	// FThumbnailPreviewScene implementation
	virtual void GetViewMatrixParameters(const float InFOVDegrees, FVector& OutOrigin, float& OutOrbitPitch, float& OutOrbitYaw, float& OutOrbitZoom) const override;

private:
	/** The static mesh actor used to display all static mesh thumbnails */
	class APrefabActor* PreviewActor;
	FBoxSphereBounds Bounds;
};


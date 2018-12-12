//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PrefabComponent.generated.h"

/** A Dungeon Theme asset lets you design the look and feel of you dungeon with an intuitive graph based approach */
UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabComponent : public USceneComponent {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	class UPrefabricatorAsset* PrefabAsset;

	virtual void OnRegister() override;

private:
	UPROPERTY()
	UTexture2D* EditorSpriteTexture;
};


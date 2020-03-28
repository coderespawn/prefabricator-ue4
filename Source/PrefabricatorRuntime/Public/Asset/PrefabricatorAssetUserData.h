//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/AssetUserData.h"
#include "PrefabricatorAssetUserData.generated.h"

UCLASS()
class PREFABRICATORRUNTIME_API UPrefabricatorAssetUserData : public UAssetUserData {
	GENERATED_BODY()

public:
	UPROPERTY(VisibleAnywhere, Category = "Prefabricator")
	TWeakObjectPtr<class APrefabActor> PrefabActor;

	UPROPERTY(VisibleAnywhere, Category = "Prefabricator")
	FGuid ItemID;
};


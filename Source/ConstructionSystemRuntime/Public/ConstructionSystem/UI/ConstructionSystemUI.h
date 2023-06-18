//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ConstructionSystemUI.generated.h"

class UConstructionSystemComponent;

UINTERFACE(Blueprintable)
class UConstructionSystemBuildUI : public UInterface
{
	GENERATED_BODY()
};

class UConstructionSystemUIAsset;

class IConstructionSystemBuildUI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Prefabricator")
	void SetUIAsset(UConstructionSystemUIAsset* UIAsset);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Prefabricator")
	void SetConstructionSystem(UConstructionSystemComponent* ConstructionSystem);
};


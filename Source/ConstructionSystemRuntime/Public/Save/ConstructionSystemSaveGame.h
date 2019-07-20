//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "ConstructionSystemSaveGame.generated.h"

class UPrefabricatorAssetInterface;

USTRUCT()
struct CONSTRUCTIONSYSTEMRUNTIME_API FConstructionSystemSaveConstructedItem {
	GENERATED_BODY()

	UPROPERTY()
	UPrefabricatorAssetInterface* PrefabAsset;

	UPROPERTY()
	int32 Seed;

	UPROPERTY()
	FTransform Transform;
};

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemSaveGame : public USaveGame {
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "ConstructionSystem")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "ConstructionSystem")
	uint32 UserIndex;

	UPROPERTY()
	TArray<FConstructionSystemSaveConstructedItem> ConstructedItems;
};

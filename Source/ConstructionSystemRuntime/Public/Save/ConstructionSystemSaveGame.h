//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "Kismet/BlueprintFunctionLibrary.h"
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

USTRUCT()
struct CONSTRUCTIONSYSTEMRUNTIME_API FConstructionSystemSavePlayerInfo {
	GENERATED_BODY()

	UPROPERTY()
	bool bRestorePlayerInfo = false;

	UPROPERTY()
	FTransform Transform = FTransform::Identity;

	UPROPERTY()
	FRotator ControlRotation = FRotator::ZeroRotator;
};

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemSaveGame : public USaveGame {
	GENERATED_BODY()
public:
	UPROPERTY(VisibleAnywhere, Category = "ConstructionSystem")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "ConstructionSystem")
	uint32 UserIndex;

	UPROPERTY(VisibleAnywhere, Category = "ConstructionSystem")
	FConstructionSystemSavePlayerInfo PlayerInfo;

	UPROPERTY()
	TArray<FConstructionSystemSaveConstructedItem> ConstructedItems;
};

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemSaveSystem : public UBlueprintFunctionLibrary {
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	static void SaveConstructionSystemLevel(const UObject* WorldContextObject, const FString& SaveSlotName, int32 UserIndex, bool bSavePlayerState);

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	static void LoadConstructionSystemLevel(const UObject* WorldContextObject, const FName& LevelName, bool bAbsolute, const FString& SaveSlotName, int32 UserIndex);

	UFUNCTION(BlueprintCallable, Category = "ConstructionSystem")
	static void HandleConstructionSystemLevelLoad(const UObject* WorldContextObject);
};


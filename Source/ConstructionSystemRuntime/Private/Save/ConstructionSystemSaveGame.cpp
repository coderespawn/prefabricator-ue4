//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Save/ConstructionSystemSaveGame.h"
#include "Engine/Engine.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"
#include "ConstructionSystemComponent.h"
#include "PrefabActor.h"
#include "PrefabricatorAsset.h"
#include "ConstructionSystemUtils.h"

void FConstructionSystemSaveSystem::SaveLevel(UWorld* InWorld, const FString& InSaveSlotName, int32 InUserIndex)
{
	if (!GEngine) return;

	UConstructionSystemSaveGame* SaveGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::CreateSaveGameObject(UConstructionSystemSaveGame::StaticClass()));
	SaveGameInstance->SaveSlotName = InSaveSlotName;
	SaveGameInstance->UserIndex = InUserIndex;
	
	for (TActorIterator<APrefabActor> It(InWorld); It; ++It) {
		APrefabActor* PrefabActor = *It;
		if (PrefabActor && PrefabActor->GetRootComponent()) {
			if (UConstructionSystemItemUserData* UserData = Cast<UConstructionSystemItemUserData>(
				PrefabActor->GetRootComponent()->GetAssetUserDataOfClass(UConstructionSystemItemUserData::StaticClass()))) {
				// This prefab actor was created using the construction system
				FConstructionSystemSaveConstructedItem Item;
				Item.PrefabAsset = PrefabActor->GetPrefabAsset();
				Item.Seed = UserData->Seed;
				Item.Transform = PrefabActor->GetActorTransform();
				SaveGameInstance->ConstructedItems.Add(Item);
			}
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}

void FConstructionSystemSaveSystem::LoadLevel(UWorld* InWorld, const FString& InSaveSlotName, int32 InUserIndex)
{
	// Destroy all the constructed items
	{
		TArray<APrefabActor*> ActorsToDestroy;
		for (TActorIterator<APrefabActor> It(InWorld); It; ++It) {
			APrefabActor* PrefabActor = *It;
			if (PrefabActor && PrefabActor->GetRootComponent()) {
				if (UConstructionSystemItemUserData* UserData = Cast<UConstructionSystemItemUserData>(
					PrefabActor->GetRootComponent()->GetAssetUserDataOfClass(UConstructionSystemItemUserData::StaticClass()))) {
					ActorsToDestroy.Add(PrefabActor);
				}
			}
		}

		for (APrefabActor* PrefabActor : ActorsToDestroy) {
			PrefabActor->Destroy();
		}
	}

	UConstructionSystemSaveGame* LoadGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::CreateSaveGameObject(UConstructionSystemSaveGame::StaticClass()));
	LoadGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::LoadGameFromSlot(InSaveSlotName, InUserIndex));

	if (LoadGameInstance) {
		for (const FConstructionSystemSaveConstructedItem& Item : LoadGameInstance->ConstructedItems) {
			FConstructionSystemUtils::ConstructPrefabItem(InWorld, Item.PrefabAsset, Item.Transform, Item.Seed);
		}
	}
}

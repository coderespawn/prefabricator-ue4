//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Save/ConstructionSystemSaveGame.h"

#include "Asset/PrefabricatorAsset.h"
#include "ConstructionSystemComponent.h"
#include "Prefab/PrefabActor.h"
#include "Utils/ConstructionSystemUtils.h"

#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/GameModeBase.h"
#include "Kismet/GameplayStatics.h"

DEFINE_LOG_CATEGORY_STATIC(LogConstructionSaveSystem, Log, All);

void UConstructionSystemSaveSystem::SaveConstructionSystemLevel(const UObject* InWorldContextObject, const FString& InSaveSlotName, int32 InUserIndex, bool bSavePlayerState)
{
	if (!GEngine) return;

	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		UE_LOG(LogConstructionSaveSystem, Error, TEXT("Failed to save level. Invalid world context object"));
		return;
	}

	UConstructionSystemSaveGame* SaveGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::CreateSaveGameObject(UConstructionSystemSaveGame::StaticClass()));
	SaveGameInstance->SaveSlotName = InSaveSlotName;
	SaveGameInstance->UserIndex = InUserIndex;
	
	for (TActorIterator<APrefabActor> It(World); It; ++It) {
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

	if (bSavePlayerState) {
		APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(InWorldContextObject, 0);
		SaveGameInstance->PlayerInfo.Transform = PlayerPawn->GetActorTransform();
		SaveGameInstance->PlayerInfo.ControlRotation = PlayerPawn->GetControlRotation();
		SaveGameInstance->PlayerInfo.bRestorePlayerInfo = true;
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, SaveGameInstance->SaveSlotName, SaveGameInstance->UserIndex);
}

void UConstructionSystemSaveSystem::LoadConstructionSystemLevel(const UObject* InWorldContextObject, const FName& InLevelName, bool bInAbsolute, const FString& InSaveSlotName, int32 InUserIndex)
{
	FString Options = FString::Printf(TEXT("CSSlot=%s?CSUserId=%d"), *InSaveSlotName, InUserIndex);
	UGameplayStatics::OpenLevel(InWorldContextObject, InLevelName, bInAbsolute, Options);
}

void UConstructionSystemSaveSystem::HandleConstructionSystemLevelLoad(const UObject* InWorldContextObject)
{
	UWorld* World = GEngine->GetWorldFromContextObject(InWorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!World) {
		UE_LOG(LogConstructionSaveSystem, Error, TEXT("Failed to load level. Invalid world context object"));
		return;
	}
	
	AGameModeBase* GameMode = World->GetAuthGameMode();
	if (!GameMode) {
		// Not the authority. Map will be loaded on the server
		return;
	}
	FString Options = GameMode->OptionsString;
	FString SaveSlotName = UGameplayStatics::ParseOption(Options, "CSSlot");
	int32 UserIndex = UGameplayStatics::GetIntOption(Options, "CSUserId", 0);

	if (SaveSlotName.Len() == 0) {
		// No load request found in the options
		return;
	}

	UConstructionSystemSaveGame* LoadGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::CreateSaveGameObject(UConstructionSystemSaveGame::StaticClass()));
	LoadGameInstance = Cast<UConstructionSystemSaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, UserIndex));

	if (LoadGameInstance) {
		for (const FConstructionSystemSaveConstructedItem& Item : LoadGameInstance->ConstructedItems) {
			FConstructionSystemUtils::ConstructPrefabItem(World, Item.PrefabAsset, Item.Transform, Item.Seed);
		}

		if (LoadGameInstance->PlayerInfo.bRestorePlayerInfo) {
			APawn* PlayerPawn = UGameplayStatics::GetPlayerPawn(InWorldContextObject, 0);

			// Teleport to the position / rotation
			FTransform PlayerTransform = LoadGameInstance->PlayerInfo.Transform;
			PlayerPawn->TeleportTo(PlayerTransform.GetLocation(), PlayerTransform.GetRotation().Rotator());

			// Set the controller rotation
			AController* Controller = PlayerPawn->GetController();
			if (Controller) {
				Controller->SetControlRotation(LoadGameInstance->PlayerInfo.ControlRotation);
			}
		}
	}
}


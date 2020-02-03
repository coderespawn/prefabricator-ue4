//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorActorFactory.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"

#include "AssetData.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorActorFactory, Log, All);

UPrefabricatorActorFactory::UPrefabricatorActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	DisplayName = NSLOCTEXT("PrefabricatorActorFactory", "PrefabricatorActorFactoryDisplayName", "Add Prefab Actor");
	NewActorClass = APrefabActor::StaticClass();
}

UObject* UPrefabricatorActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(ActorInstance);
	if (PrefabActor) {
		return PrefabActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous();
	}
	return nullptr;
}

AActor* UPrefabricatorActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	AActor* Actor = UActorFactory::SpawnActor(Asset, InLevel, Transform, InObjectFlags, Name);
	APrefabActor* PrefabActor = Cast<APrefabActor>(Actor);
	if (PrefabActor) {
		PrefabActor->PrefabComponent->PrefabAssetInterface = Cast<UPrefabricatorAssetInterface>(Asset);
	}
	return Actor;
}

void UPrefabricatorActorFactory::LoadPrefabActorState(APrefabActor* PrefabActor)
{
	if (PrefabActor) {
		bool bIsPreviewActor  = (PrefabActor->GetFlags() & RF_Transient) > 0;

		FPrefabLoadSettings LoadSettings;
		LoadSettings.bUnregisterComponentsBeforeLoading = !bIsPreviewActor;
		FPrefabTools::LoadStateFromPrefabAsset(PrefabActor, LoadSettings);
		if (bIsPreviewActor) {
			FPrefabTools::IterateChildrenRecursive(PrefabActor, [](AActor* ChildActor) {
				ChildActor->SetActorEnableCollision(false);
			});
		}
	}
}

void UPrefabricatorActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(NewActor);

	if (PrefabActor && PrefabActor->PrefabComponent) {
		PrefabActor->PrefabComponent->PrefabAssetInterface = Cast<UPrefabricatorAssetInterface>(Asset);
		
		LoadPrefabActorState(PrefabActor);
	}

}

void UPrefabricatorActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(CDO);
	if (PrefabActor && PrefabActor->PrefabComponent) {
		PrefabActor->PrefabComponent->PrefabAssetInterface = Cast<UPrefabricatorAssetInterface>(Asset);

		LoadPrefabActorState(PrefabActor);
	}
	UE_LOG(LogPrefabricatorActorFactory, Log, TEXT("Call: UPrefabricatorActorFactory::PostCreateBlueprint"));
}

bool UPrefabricatorActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(UPrefabricatorAssetInterface::StaticClass())) {
		return true;
	}
	else {
		OutErrorMsg = NSLOCTEXT("PrefabricatorActorFactory", "PrefabricatorActorFactoryError", "No valid prefab asset was specified");
		return false;
	}
}


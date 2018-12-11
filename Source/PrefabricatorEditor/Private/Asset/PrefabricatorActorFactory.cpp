//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorActorFactory.h"

#include "PrefabActor.h"
#include "PrefabComponent.h"
#include "PrefabricatorAsset.h"

#include "AssetData.h"
#include "PrefabEditorTools.h"

UPrefabricatorActorFactory::UPrefabricatorActorFactory(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	DisplayName = NSLOCTEXT("PrefabricatorActorFactory", "PrefabricatorActorFactoryDisplayName", "Add Prefab Actor");
	NewActorClass = APrefabActor::StaticClass();
}

UObject* UPrefabricatorActorFactory::GetAssetFromActorInstance(AActor* ActorInstance)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(ActorInstance);
	return PrefabActor ? PrefabActor->PrefabComponent->PrefabAsset : nullptr;
}

AActor* UPrefabricatorActorFactory::SpawnActor(UObject* Asset, ULevel* InLevel, const FTransform& Transform, EObjectFlags InObjectFlags, const FName Name)
{
	AActor* Actor = UActorFactory::SpawnActor(Asset, InLevel, Transform, InObjectFlags, Name);
	APrefabActor* PrefabActor = Cast<APrefabActor>(Actor);
	if (PrefabActor) {
		PrefabActor->PrefabComponent->PrefabAsset = Cast<UPrefabricatorAsset>(Asset);
		FPrefabEditorTools::LoadStateFromPrefabAsset(PrefabActor);
	}
	return Actor;
}

void UPrefabricatorActorFactory::PostSpawnActor(UObject* Asset, AActor* NewActor)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(NewActor);
	if (PrefabActor && PrefabActor->PrefabComponent) {
		PrefabActor->PrefabComponent->PrefabAsset = Cast<UPrefabricatorAsset>(Asset);
		FPrefabEditorTools::LoadStateFromPrefabAsset(PrefabActor);
	}
}

void UPrefabricatorActorFactory::PostCreateBlueprint(UObject* Asset, AActor* CDO)
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(CDO);
	if (PrefabActor && PrefabActor->PrefabComponent) {
		PrefabActor->PrefabComponent->PrefabAsset = Cast<UPrefabricatorAsset>(Asset);
		FPrefabEditorTools::LoadStateFromPrefabAsset(PrefabActor);
	}
}

bool UPrefabricatorActorFactory::CanCreateActorFrom(const FAssetData& AssetData, FText& OutErrorMsg)
{
	if (AssetData.IsValid() && AssetData.GetClass()->IsChildOf(UPrefabricatorAsset::StaticClass())) {
		return true;
	}
	else {
		OutErrorMsg = NSLOCTEXT("PrefabricatorActorFactory", "PrefabricatorActorFactoryError", "No valid prefab asset was specified");
		return false;
	}
}


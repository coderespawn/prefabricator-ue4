//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Tools/PrefabEditorTools.h"

#include "PrefabActor.h"
#include "PrefabricatorAsset.h"
#include "PrefabricatorAssetUserData.h"

#include "Editor/EditorEngine.h"
#include "Engine/Selection.h"
#include "GameFramework/Actor.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "IContentBrowserSingleton.h"
#include "PrefabComponent.h"
#include "MemoryReader.h"
#include "ObjectAndNameAsStringProxyArchive.h"
#include "EngineUtils.h"
#include "ObjectWriter.h"
#include "ObjectReader.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabEditorTools, Log, All);

bool FPrefabEditorTools::CanCreatePrefab()
{
	if (!GEditor) {
		return false;
	}

	int32 NumSelected = GEditor->GetSelectedActorCount();
	return NumSelected > 0;
}

void FPrefabEditorTools::CreatePrefab()
{
	TArray<AActor*> SelectedActors;
	GetSelectedActors(SelectedActors);

	CreatePrefabFromActors(SelectedActors);
}

void FPrefabEditorTools::CreatePrefabFromActors(const TArray<AActor*>& Actors)
{
	if (Actors.Num() == 0) {
		return;
	}

	UWorld* World = Actors[0]->GetWorld();

	FVector Pivot = FPrefabricatorAssetUtils::FindPivot(Actors);
	APrefabActor* PrefabActor = World->SpawnActor<APrefabActor>(Pivot, FRotator::ZeroRotator);

	// Find the compatible mobility for the prefab actor
	EComponentMobility::Type Mobility = FPrefabricatorAssetUtils::FindMobility(Actors);
	PrefabActor->GetRootComponent()->SetMobility(Mobility);

	UPrefabricatorAsset* PrefabAsset = CreatePrefabAsset();
	PrefabActor->PrefabComponent->PrefabAsset = PrefabAsset;

	// Attach the actors to the prefab
	if (GEditor) {
		for (AActor* Actor : Actors) {
			if (Actor->GetRootComponent()) {
				Actor->GetRootComponent()->SetMobility(Mobility);
			}

			GEditor->ParentActors(PrefabActor, Actor, NAME_None);
		}
	}

	SaveStateToPrefabAsset(PrefabActor);

	if (GEditor) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(PrefabActor, true, true);
	}
}

void FPrefabEditorTools::GetSelectedActors(TArray<AActor*>& OutActors)
{
	if (GEditor) {
		USelection* SelectedActors = GEditor->GetSelectedActors();
		for (FSelectionIterator Iter(*SelectedActors); Iter; ++Iter)
		{
			// We only care about actors that are referenced in the world for literals, and also in the same level as this blueprint
			AActor* Actor = Cast<AActor>(*Iter);
			if (Actor) 
			{
				OutActors.Add(Actor);
			}
		}
	}
}

void FPrefabEditorTools::AssignAssetUserData(AActor* InActor, APrefabActor* Prefab)
{
	if (!InActor || !InActor->GetRootComponent()) {
		return;
	}
	
	UPrefabricatorAssetUserData* PrefabUserData = NewObject<UPrefabricatorAssetUserData>(InActor->GetRootComponent());
	PrefabUserData->PrefabActor = Prefab;
	InActor->GetRootComponent()->AddAssetUserData(PrefabUserData);
}

UPrefabricatorAsset* FPrefabEditorTools::CreatePrefabAsset()
{
	IContentBrowserSingleton& ContentBrowserSingleton = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser").Get();
	TArray<FString> SelectedFolders;
	ContentBrowserSingleton.GetSelectedPathViewFolders(SelectedFolders);
	FString PrefabFolder = SelectedFolders.Num() > 0 ? SelectedFolders[0] : "/Game";
	FString PrefabPath = PrefabFolder + "/Prefab";

	FString PackageName, AssetName;
	IAssetTools& AssetTools = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools").Get();
	AssetTools.CreateUniqueAssetName(*PrefabPath, TEXT(""), PackageName, AssetName);
	UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(AssetTools.CreateAsset(AssetName, PrefabFolder, UPrefabricatorAsset::StaticClass(), nullptr));

	ContentBrowserSingleton.SyncBrowserToAssets(TArray<UObject*>({ PrefabAsset }));

	return PrefabAsset;
}



void FPrefabEditorTools::SaveStateToPrefabAsset(APrefabActor* PrefabActor)
{
	if (!PrefabActor) {
		UE_LOG(LogPrefabEditorTools, Error, TEXT("Invalid prefab actor reference"));
		return;
	}

	UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
	if (!PrefabAsset) {
		UE_LOG(LogPrefabEditorTools, Error, TEXT("Prefab asset is not assigned correctly"));
		return;
	}

	PrefabAsset->ActorData.Reset();

	FTransform InvPrefabTransform = PrefabActor->GetTransform().Inverse();
	TArray<AActor*> Children;
	GetActorChildren(PrefabActor, Children);

	for (AActor* ChildActor : Children) {
		AssignAssetUserData(ChildActor, PrefabActor);
		int32 NewItemIndex = PrefabAsset->ActorData.AddDefaulted();
		FPrefabricatorActorData& ActorData = PrefabAsset->ActorData[NewItemIndex];
		SaveStateToPrefabAsset(ChildActor, InvPrefabTransform, ActorData);
	}
}

void FPrefabEditorTools::SaveStateToPrefabAsset(AActor* InActor, const FTransform& InversePrefabTransform, FPrefabricatorActorData& OutActorData)
{
	FTransform LocalTransform = InActor->GetTransform() * InversePrefabTransform;
	OutActorData.RelativeTransform = LocalTransform;
	OutActorData.ActorClass = InActor->GetClass();

	for (UActorComponent* Component : InActor->GetComponents()) {
		FPrefabricatorComponentData ComponentItemData;
		ComponentItemData.ComponentName = Component->GetFName();
		FObjectWriter(InActor, ComponentItemData.Data);
		OutActorData.ComponentData.Add(ComponentItemData);
	}
}

void FPrefabEditorTools::GetActorChildren(AActor* InParent, TArray<AActor*>& OutChildren)
{
	InParent->GetAttachedActors(OutChildren);
}

void FPrefabEditorTools::LoadStateFromPrefabAsset(APrefabActor* PrefabActor)
{
	if (!PrefabActor) {
		UE_LOG(LogPrefabEditorTools, Error, TEXT("Invalid prefab actor reference"));
		return;
	}

	UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
	if (!PrefabAsset) {
		UE_LOG(LogPrefabEditorTools, Error, TEXT("Prefab asset is not assigned correctly"));
		return;
	}

	TMap<UClass*, TArray<AActor*>> ExistingActorPool;

	TArray<AActor*> Children;
	GetActorChildren(PrefabActor, Children);

	for (AActor* ChildActor : Children) {
		TArray<AActor*>& ActorsByClass = ExistingActorPool.FindOrAdd(ChildActor->GetClass());
		ActorsByClass.Add(ChildActor);
	}

	for (FPrefabricatorActorData& ActorItemData : PrefabAsset->ActorData) {
		if (!ActorItemData.ActorClass) return;

		UWorld* World = PrefabActor->GetWorld();
		AActor* ChildActor = nullptr;
		TArray<AActor*>& ActorPoolByClass = ExistingActorPool.FindOrAdd(ActorItemData.ActorClass);
		if (ActorPoolByClass.Num() > 0) {
			ChildActor = ActorPoolByClass[0];
			ActorPoolByClass.RemoveAt(0);
		}
		else {
			// The pool is exhausted for this class type. Spawn a new actor
			ChildActor = World->SpawnActor<AActor>(ActorItemData.ActorClass);
			ChildActor->AttachToActor(PrefabActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
		}


		// Load the saved data into the actor
		{
			TMap<FName, FPrefabricatorComponentData*> ComponentDataByName;
			for (FPrefabricatorComponentData& ComponentData : ActorItemData.ComponentData) {
				if (!ComponentDataByName.Contains(ComponentData.ComponentName)) {
					ComponentDataByName.Add(ComponentData.ComponentName, &ComponentData);
				}
			}

			for (UActorComponent* Component : ChildActor->GetComponents()) {
				FName ComponentName = Component->GetFName();
				if (ComponentDataByName.Contains(ComponentName)) {
					FPrefabricatorComponentData& ComponentData = *ComponentDataByName[ComponentName];
					FObjectReader(Component, ComponentData.Data);
					Component->ReregisterComponent();
				}
			}
		}

		AssignAssetUserData(ChildActor, PrefabActor);

		// Set the transform
		FTransform WorldTransform = ActorItemData.RelativeTransform * PrefabActor->GetTransform();
		ChildActor->SetActorTransform(WorldTransform);
	}

	// Delete the unused actors from the pool
	for (auto& Entry : ExistingActorPool) {
		TArray<AActor*>& ActorsByClass = Entry.Value;
		for (AActor* Actor : ActorsByClass) {
			if (Actor && Actor->GetRootComponent()) {
				UPrefabricatorAssetUserData* PrefabUserData = Actor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
				if (PrefabUserData && PrefabUserData->PrefabActor == PrefabActor) {
					Actor->Destroy();
				}
			}
		}
	}
	ExistingActorPool.Reset();

}


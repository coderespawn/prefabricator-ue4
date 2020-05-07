//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorEditorService.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Utils/PrefabEditorTools.h"

#include "ActorFactories/ActorFactory.h"
#include "ActorFactories/ActorFactoryBoxVolume.h"
#include "AssetToolsModule.h"
#include "ComponentAssetBroker.h"
#include "Editor.h"
#include "Editor/EditorEngine.h"
#include "EditorViewportClient.h"
#include "Engine/Selection.h"
#include "LevelEditor.h"
#include "LevelEditorViewport.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "ThumbnailRendering/SceneThumbnailInfo.h"

void FPrefabricatorEditorService::ParentActors(AActor* ParentActor, AActor* ChildActor)
{
	/*
	if (GEditor) {
		GEditor->ParentActors(ParentActor, ChildActor, NAME_None);
	}
	*/

	USceneComponent* ChildRoot = ChildActor->GetRootComponent();
	USceneComponent* ParentRoot = ParentActor->GetDefaultAttachComponent();

	check(ChildRoot);	// CanParentActors() call should ensure this
	check(ParentRoot);	// CanParentActors() call should ensure this

	// modify parent and child
	// TODO: Check if we need to modify
	ChildActor->Modify();
	ParentActor->Modify();

	// If child is already attached to something, modify the old parent and detach
	if (ChildRoot->GetAttachParent() != nullptr)
	{
		AActor* OldParentActor = ChildRoot->GetAttachParent()->GetOwner();
		OldParentActor->Modify();
		ChildRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// If the parent is already attached to this child, modify its parent and detach so we can allow the attachment
	if (ParentRoot->IsAttachedTo(ChildRoot))
	{
		ParentRoot->GetAttachParent()->GetOwner()->Modify();
		ParentRoot->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
	}

	// Snap to socket if a valid socket name was provided, otherwise attach without changing the relative transform
	ChildRoot->AttachToComponent(ParentRoot, FAttachmentTransformRules::KeepWorldTransform, NAME_None);
}

void FPrefabricatorEditorService::SelectPrefabActor(AActor* PrefabActor)
{
	if (GEditor) {
		GEditor->SelectNone(true, true);
		GEditor->SelectActor(PrefabActor, true, true);
	}
}

void FPrefabricatorEditorService::GetSelectedActors(TArray<AActor*>& OutActors)
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

int FPrefabricatorEditorService::GetNumSelectedActors()
{
	return GEditor ? GEditor->GetSelectedActorCount() : 0;
}

UPrefabricatorAsset* FPrefabricatorEditorService::CreatePrefabAsset()
{
	return FPrefabEditorTools::CreatePrefabAsset();
}

FVector FPrefabricatorEditorService::SnapToGrid(const FVector& InLocation)
{
	auto& Settings = GetDefault<ULevelEditorViewportSettings>()->SnapToSurface;
	if (GEditor) {
		float GridSize = GEditor->GetGridSize();
#define PF_SNAP_TO_GRID(X) FMath::RoundToInt((X) / GridSize) * GridSize
		FVector SnappedLocation(
			PF_SNAP_TO_GRID(InLocation.X),
			PF_SNAP_TO_GRID(InLocation.Y),
			PF_SNAP_TO_GRID(InLocation.Z));
#undef PF_SNAP_TO_GRID
		return SnappedLocation;
	}
	else {
		return InLocation;
	}
}

void FPrefabricatorEditorService::SetDetailsViewObject(UObject* InObject)
{
	if (!InObject) return;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	static const FName LevelEditorDetailsId = TEXT("LevelEditorSelectionDetails");
	TSharedPtr<IDetailsView> LevelEditorDetailsView = PropertyEditorModule.FindDetailView(LevelEditorDetailsId);
	if (LevelEditorDetailsView.IsValid()) {
		LevelEditorDetailsView->SetObject(InObject, true);
	}
}


void FPrefabricatorEditorService::RefreshDetailsViewObject(UObject* InObject)
{
	if (!InObject) return;

	FPropertyEditorModule& PropertyEditorModule = FModuleManager::GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	static const FName LevelEditorDetailsId = TEXT("LevelEditorSelectionDetails");
	TSharedPtr<IDetailsView> LevelEditorDetailsView = PropertyEditorModule.FindDetailView(LevelEditorDetailsId);
	if (LevelEditorDetailsView.IsValid()) {
		TArray<TWeakObjectPtr<UObject>> CurrentSelection = LevelEditorDetailsView->GetSelectedObjects();
		if (CurrentSelection.Num() == 1 && CurrentSelection[0].Get() == InObject) {
			LevelEditorDetailsView->SetObject(InObject, true);
		}
	}
}

AActor* FPrefabricatorEditorService::SpawnActor(TSubclassOf<AActor> InActorClass, const FTransform& InTransform, ULevel* InLevel, AActor* InTemplate)
{
	if (GEditor && !InTemplate) {
		UActorFactory* ActorFactory = GEditor->FindActorFactoryByClassForActorClass(UActorFactoryBoxVolume::StaticClass(), InActorClass);
		if (ActorFactory) {
			FAssetData AssetData(InActorClass);
			return ActorFactory->CreateActor(AssetData.GetAsset(), InLevel, InTransform);
		}
	}

	return IPrefabricatorService::SpawnActor(InActorClass, InTransform, InLevel, InTemplate);
}

namespace {
	/**
	 * @return		The shared transaction object used by
	 */
	static FScopedTransaction*& PrefabStaticTransaction()
	{
		static FScopedTransaction* STransaction = NULL;
		return STransaction;
	}

	/**
	 * Ends the outstanding transaction, if one exists.
	 */
	static void PrefabEndTransaction()
	{
		delete PrefabStaticTransaction();
		PrefabStaticTransaction() = NULL;
	}

	/**
	 * Begins a new transaction, if no outstanding transaction exists.
	 */
	static void PrefabBeginTransaction(const FText& Description)
	{
		if (!PrefabStaticTransaction())
		{
			PrefabStaticTransaction() = new FScopedTransaction(Description);
		}
	}
} // namespace


void FPrefabricatorEditorService::BeginTransaction(const FText& Description)
{
	PrefabBeginTransaction(Description);
}

void FPrefabricatorEditorService::EndTransaction()
{
	PrefabEndTransaction();
}

void FPrefabricatorEditorService::RunGC()
{
	CollectGarbage(GARBAGE_COLLECTION_KEEPFLAGS);
}

void FPrefabricatorEditorService::CaptureThumb(UPrefabricatorAsset* PrefabAsset)
{
	// Save the thumbnail
	FPrefabEditorTools::CapturePrefabAssetThumbnail(PrefabAsset);
}


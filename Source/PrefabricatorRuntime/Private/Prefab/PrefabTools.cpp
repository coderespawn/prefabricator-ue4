//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/PrefabTools.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAssetUserData.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabComponent.h"
#include "Utils/PrefabricatorService.h"

#include "Engine/Selection.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "HAL/UnrealMemory.h"
#include "PropertyPathHelpers.h"
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabTools, Log, All);

#define LOCTEXT_NAMESPACE "PrefabTools"

void FPrefabTools::GetSelectedActors(TArray<AActor*>& OutActors)
{
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Service->GetSelectedActors(OutActors);
	}
}


int FPrefabTools::GetNumSelectedActors()
{
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	return Service.IsValid() ? Service->GetNumSelectedActors() : 0;
}

void FPrefabTools::ParentActors(AActor* ParentActor, AActor* ChildActor)
{
	if (ChildActor && ParentActor) {
		ChildActor->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
		TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
		if (Service.IsValid()) {
			Service->ParentActors(ParentActor, ChildActor);
		}
	}
}

void FPrefabTools::SelectPrefabActor(AActor* PrefabActor)
{
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Service->SelectPrefabActor(PrefabActor);
	}
}

UPrefabricatorAsset* FPrefabTools::CreatePrefabAsset()
{
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	return Service.IsValid() ? Service->CreatePrefabAsset() : nullptr;
}

int32 FPrefabTools::GetRandomSeed(const FRandomStream& InRandom)
{
	return InRandom.RandRange(0, 10000000);
}

void FPrefabTools::IterateChildrenRecursive(APrefabActor* Prefab, TFunction<void(AActor*)> Visit)
{
	TArray<AActor*> Stack;
	{
		TArray<AActor*> AttachedActors;
		Prefab->GetAttachedActors(AttachedActors);
		for (AActor* Child : AttachedActors) {
			Stack.Push(Child);
		}
	}

	while (Stack.Num() > 0) {
		AActor* Top = Stack.Pop();

		Visit(Top);

		{
			TArray<AActor*> AttachedActors;
			Top->GetAttachedActors(AttachedActors);
			for (AActor* Child : AttachedActors) {
				Stack.Push(Child);
			}
		}
	}
}

bool FPrefabTools::CanCreatePrefab()
{
	return GetNumSelectedActors() > 0;
}

void FPrefabTools::CreatePrefab()
{
	TArray<AActor*> SelectedActors;
	GetSelectedActors(SelectedActors);

	CreatePrefabFromActors(SelectedActors);
}

namespace {

	void SanitizePrefabActorsForCreation(const TArray<AActor*>& InActors, TArray<AActor*>& OutActors) {
		// Find all the selected prefab actors
		TArray<APrefabActor*> PrefabActors;
		for (AActor* Actor : InActors) {
			if (APrefabActor* PrefabActor = Cast<APrefabActor>(Actor)) {
				PrefabActors.Add(PrefabActor);
			}
		}

		// Make sure we do not include any actors that belong to these prefabs
		for (AActor* Actor : InActors) {
			bool bValid = true;
			if (APrefabActor* ParentPrefab = Cast<APrefabActor>(Actor->GetAttachParentActor())) {
				if (PrefabActors.Contains(ParentPrefab)) {
					bValid = false;
				}
			}

			if (bValid) {
				OutActors.Add(Actor);
			}
		}
	}

}
void FPrefabTools::CreatePrefabFromActors(const TArray<AActor*>& InActors)
{
	TArray<AActor*> Actors;
	SanitizePrefabActorsForCreation(InActors, Actors);

	if (Actors.Num() == 0) {
		return;
	}

	UPrefabricatorAsset* PrefabAsset = CreatePrefabAsset();
	if (!PrefabAsset) {
		return;
	}

	UWorld* World = Actors[0]->GetWorld();

	FVector Pivot = FPrefabricatorAssetUtils::FindPivot(Actors);
	APrefabActor* PrefabActor = World->SpawnActor<APrefabActor>(Pivot, FRotator::ZeroRotator);

	// Find the compatible mobility for the prefab actor
	EComponentMobility::Type Mobility = FPrefabricatorAssetUtils::FindMobility(Actors);
	PrefabActor->GetRootComponent()->SetMobility(Mobility);

	PrefabActor->PrefabComponent->PrefabAssetInterface = PrefabAsset;
	// Attach the actors to the prefab
	for (AActor* Actor : Actors) {
		if (Actor->GetRootComponent()) {
			Actor->GetRootComponent()->SetMobility(Mobility);
		}
		ParentActors(PrefabActor, Actor);
	}

	SaveStateToPrefabAsset(PrefabActor);

	SelectPrefabActor(PrefabActor);
}

void FPrefabTools::AssignAssetUserData(AActor* InActor, const FGuid& InItemID, APrefabActor* Prefab)
{
	if (!InActor || !InActor->GetRootComponent()) {
		return;
	}
	
	UPrefabricatorAssetUserData* PrefabUserData = NewObject<UPrefabricatorAssetUserData>(InActor->GetRootComponent());
	PrefabUserData->PrefabActor = Prefab;
	PrefabUserData->ItemID = InItemID;
	InActor->GetRootComponent()->AddAssetUserData(PrefabUserData);
}


void FPrefabTools::SaveStateToPrefabAsset(APrefabActor* PrefabActor)
{
	if (!PrefabActor) {
		UE_LOG(LogPrefabTools, Error, TEXT("Invalid prefab actor reference"));
		return;
	}

	UPrefabricatorAsset* PrefabAsset = PrefabAsset = Cast<UPrefabricatorAsset>(PrefabActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous());
	if (!PrefabAsset) {
		//UE_LOG(LogPrefabTools, Error, TEXT("Prefab asset is not assigned correctly"));
		return;
	}

	PrefabAsset->PrefabMobility = PrefabActor->GetRootComponent()->Mobility;

	PrefabAsset->ActorData.Reset();

	TArray<AActor*> Children;
	GetActorChildren(PrefabActor, Children);

	// Make sure the children do not have duplicate asset user data template ids
	{
		TSet<FGuid> VisitedItemId;
		for (AActor* ChildActor : Children) {
			if (ChildActor && ChildActor->GetRootComponent()) {
				UPrefabricatorAssetUserData* ChildUserData = ChildActor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
				if (ChildUserData) {
					if (VisitedItemId.Contains(ChildUserData->ItemID)) {
						ChildUserData->ItemID = FGuid::NewGuid();
						ChildUserData->Modify();
					}
					VisitedItemId.Add(ChildUserData->ItemID);
				}
			}
		}
	}

	for (AActor* ChildActor : Children) {
		if (ChildActor && ChildActor->GetRootComponent()) {
			UPrefabricatorAssetUserData* ChildUserData = ChildActor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
			FGuid ItemID;
			if (ChildUserData && ChildUserData->PrefabActor == PrefabActor) {
				ItemID = ChildUserData->ItemID;
			}
			else {
				ItemID = FGuid::NewGuid();
			}
			
			AssignAssetUserData(ChildActor, ItemID, PrefabActor);
			int32 NewItemIndex = PrefabAsset->ActorData.AddDefaulted();
			FPrefabricatorActorData& ActorData = PrefabAsset->ActorData[NewItemIndex];
			ActorData.PrefabItemID = ItemID;
			SaveStateToPrefabAsset(ChildActor, PrefabActor, ActorData);
		}
	}

	PrefabActor->PrefabComponent->UpdateBounds();

	// Regenerate a new update id for the prefab asset
	PrefabAsset->LastUpdateID = FGuid::NewGuid();
	PrefabActor->LastUpdateID = PrefabAsset->LastUpdateID;

	PrefabAsset->Modify();
}

namespace {
	void GetPropertyData(UProperty* Property, UObject* Obj, FString& OutPropertyData) {
		Property->ExportTextItem(OutPropertyData, Property->ContainerPtrToValuePtr<void>(Obj), nullptr, Obj, PPF_None);
	}

	bool ContainsOuterParent(UObject* ObjectToTest, UObject* Outer) {
		while (ObjectToTest) {
			if (ObjectToTest == Outer) return true;
			ObjectToTest = ObjectToTest->GetOuter();
		}
		return false;
	}

	bool HasDefaultValue(UObject* InContainer, const FString& InPropertyPath) {
		if (!InContainer) return false;

		UClass* ObjClass = InContainer->GetClass();
		if (!ObjClass) return false;
		UObject* DefaultObject = ObjClass->GetDefaultObject();

		FString PropertyValue, DefaultValue;
		PropertyPathHelpers::GetPropertyValueAsString(InContainer, InPropertyPath, PropertyValue);
		PropertyPathHelpers::GetPropertyValueAsString(DefaultObject, InPropertyPath, DefaultValue);
		return PropertyValue == DefaultValue;
	}

	bool ShouldSkipSerialization(const UProperty* Property, UObject* ObjToSerialize, APrefabActor* PrefabActor) {
		if (const UObjectProperty* ObjProperty = Cast<const UObjectProperty>(Property)) {
			UObject* PropertyObjectValue = ObjProperty->GetObjectPropertyValue_InContainer(ObjToSerialize);
			if (ContainsOuterParent(PropertyObjectValue, ObjToSerialize) ||
				ContainsOuterParent(PropertyObjectValue, PrefabActor)) {
				//UE_LOG(LogPrefabTools, Log, TEXT("Skipping Property: %s"), *Property->GetName());
				return true;
			}
		}

		return false;
	}

	void DeserializeFields(UObject* InObjToDeserialize, const TArray<UPrefabricatorProperty*>& InProperties) {
		TMap<FString, UPrefabricatorProperty*> PropertiesByName;
		for (UPrefabricatorProperty* Property : InProperties) {
			if (!Property) continue;
			PropertiesByName.Add(Property->PropertyName, Property);
		}

		for (TFieldIterator<UProperty> PropertyIterator(InObjToDeserialize->GetClass()); PropertyIterator; ++PropertyIterator) {
			UProperty* Property = *PropertyIterator;
			if (!Property) continue;

			FString PropertyName = Property->GetName();
			UPrefabricatorProperty** SearchResult = PropertiesByName.Find(PropertyName);
			if (!SearchResult) continue;

			UPrefabricatorProperty* PrefabProperty = *SearchResult;
			if (PrefabProperty) {
				PropertyPathHelpers::SetPropertyValueFromString(InObjToDeserialize, PrefabProperty->PropertyName, PrefabProperty->ExportedValue);
			}
		}
	}

	void SerializeFields(UObject* ObjToSerialize, APrefabActor* PrefabActor, TArray<UPrefabricatorProperty*>& OutProperties) {

		UPrefabricatorAsset* PrefabAsset = Cast<UPrefabricatorAsset>(PrefabActor->PrefabComponent->PrefabAssetInterface.LoadSynchronous());

		if (!PrefabAsset) {
			return;
		}

		TSet<const UProperty*> PropertiesToSerialize;
		for (TFieldIterator<UProperty> PropertyIterator(ObjToSerialize->GetClass()); PropertyIterator; ++PropertyIterator) {
			UProperty* Property = *PropertyIterator;
			if (!Property) continue;
			if (Property->HasAnyPropertyFlags(CPF_Transient) || !Property->HasAnyPropertyFlags(CPF_Edit | CPF_Interp)) {
				continue;
			}

			if (FPrefabTools::ShouldIgnorePropertySerialization(Property->GetFName())) {
				continue;
			}

			// Check if it has the default value
			FString PropertyName = Property->GetName();
			if (HasDefaultValue(ObjToSerialize, PropertyName)) {
				continue;
			}

			PropertiesToSerialize.Add(Property);
		}

		for (const UProperty* Property : PropertiesToSerialize) {
			if (!Property) continue;
			if (FPrefabTools::ShouldIgnorePropertySerialization(Property->GetFName())) {
				continue;
			}

			UPrefabricatorProperty* PrefabProperty = nullptr;
			FString PropertyName = Property->GetName();

			if (ShouldSkipSerialization(Property, ObjToSerialize, PrefabActor)) {
				continue;
			}

			PrefabProperty = NewObject<UPrefabricatorProperty>(PrefabAsset);
			PrefabProperty->PropertyName = PropertyName;
			PropertyPathHelpers::GetPropertyValueAsString(ObjToSerialize, PropertyName, PrefabProperty->ExportedValue);
			OutProperties.Add(PrefabProperty);
		}
	}

	void CollectAllSubobjects(UObject* Object, TArray<UObject*>& OutSubobjectArray)
	{
		const bool bIncludedNestedObjects = true;
		GetObjectsWithOuter(Object, OutSubobjectArray, bIncludedNestedObjects);

		// Remove contained objects that are not subobjects.
		for (int32 ComponentIndex = 0; ComponentIndex < OutSubobjectArray.Num(); ComponentIndex++)
		{
			UObject* PotentialComponent = OutSubobjectArray[ComponentIndex];
			if (!PotentialComponent->IsDefaultSubobject() && !PotentialComponent->HasAnyFlags(RF_DefaultSubObject))
			{
				OutSubobjectArray.RemoveAtSwap(ComponentIndex--);
			}
		}
	}

	void DumpSerializedProperties(const TArray<UPrefabricatorProperty*>& InProperties) {
		for (UPrefabricatorProperty* Property : InProperties) {
			UE_LOG(LogPrefabTools, Log, TEXT("%s: %s"), *Property->PropertyName, *Property->ExportedValue);
		}

	}

	void DumpSerializedData(const FPrefabricatorActorData& InActorData) {
		UE_LOG(LogPrefabTools, Log, TEXT("Actor Properties: %s"), *InActorData.ClassPath);
		UE_LOG(LogPrefabTools, Log, TEXT("================="));
		DumpSerializedProperties(InActorData.Properties);

		for (const FPrefabricatorComponentData& ComponentData : InActorData.Components) {
			UE_LOG(LogPrefabTools, Log, TEXT(""));
			UE_LOG(LogPrefabTools, Log, TEXT("Component Properties: %s"), *ComponentData.ComponentName);
			UE_LOG(LogPrefabTools, Log, TEXT("================="));
			DumpSerializedProperties(ComponentData.Properties);
		}
	}
}

bool FPrefabTools::ShouldIgnorePropertySerialization(const FName& InPropertyName)
{
	static const TSet<FName> IgnoredFields = {
		"AttachParent",
		"AttachSocketName",
		"AttachChildren",
		"ClientAttachedChildren",
		"bIsEditorPreviewActor",
		"bIsEditorOnlyActor"
	};

	return IgnoredFields.Contains(InPropertyName);
}

void FPrefabTools::SaveStateToPrefabAsset(AActor* InActor, APrefabActor* PrefabActor, FPrefabricatorActorData& OutActorData)
{
	if (!InActor) return;

	FTransform InversePrefabTransform = PrefabActor->GetTransform().Inverse();
	FTransform LocalTransform = InActor->GetTransform() * InversePrefabTransform;
	OutActorData.RelativeTransform = LocalTransform;
	OutActorData.ClassPath = InActor->GetClass()->GetPathName();
	SerializeFields(InActor, PrefabActor, OutActorData.Properties);

	TArray<UActorComponent*> Components;
	InActor->GetComponents(Components);

	for (UActorComponent* Component : Components) {
		int32 ComponentDataIdx = OutActorData.Components.AddDefaulted();
		FPrefabricatorComponentData& ComponentData = OutActorData.Components[ComponentDataIdx];
		ComponentData.ComponentName = Component->GetPathName(InActor);
		if (USceneComponent* SceneComponent = Cast<USceneComponent>(Component)) {
			ComponentData.RelativeTransform = SceneComponent->GetComponentTransform();
		}
		else {
			ComponentData.RelativeTransform = FTransform::Identity;
		}
		SerializeFields(Component, PrefabActor, ComponentData.Properties);
	}

	DumpSerializedData(OutActorData);
}

void FPrefabTools::LoadStateFromPrefabAsset(AActor* InActor, const FPrefabricatorActorData& InActorData, const FPrefabLoadSettings& InSettings)
{

	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Service->BeginTransaction(LOCTEXT("TransLabel_LoadPrefab", "Load Prefab"));
	}

	DeserializeFields(InActor, InActorData.Properties);

	TMap<FString, UActorComponent*> ComponentsByName;
	for (UActorComponent* Component : InActor->GetComponents()) {
		FString ComponentPath = Component->GetPathName(InActor);
		ComponentsByName.Add(ComponentPath, Component);
	}

	for (const FPrefabricatorComponentData& ComponentData : InActorData.Components) {
		if (UActorComponent** SearchResult = ComponentsByName.Find(ComponentData.ComponentName)) {
			UActorComponent* Component = *SearchResult;
			bool bPreviouslyRegister = Component->IsRegistered();
			if (InSettings.bUnregisterComponentsBeforeLoading && bPreviouslyRegister) {
				Component->UnregisterComponent();
			}

			DeserializeFields(Component, ComponentData.Properties);

			if (InSettings.bUnregisterComponentsBeforeLoading && bPreviouslyRegister) {
				Component->RegisterComponent();
			}
		}
	}

	if (Service.IsValid()) {
		Service->EndTransaction();
	}

}

void FPrefabTools::GetActorChildren(AActor* InParent, TArray<AActor*>& OutChildren)
{
	InParent->GetAttachedActors(OutChildren);
}

namespace {
	void GetPrefabBoundsRecursive(AActor* InActor, FBox& OutBounds) {
		if (!InActor->IsA<APrefabActor>()) {
			FBox ActorBounds = InActor->GetComponentsBoundingBox(false);
			if (ActorBounds.GetExtent() == FVector::ZeroVector) {
				ActorBounds = FBox({ InActor->GetActorLocation() });
			}
			OutBounds += ActorBounds;
		}

		TArray<AActor*> AttachedActors;
		InActor->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors) {
			GetPrefabBoundsRecursive(AttachedActor, OutBounds);
		}
	}
}

FBox FPrefabTools::GetPrefabBounds(AActor* PrefabActor)
{
	FBox Result(EForceInit::ForceInit);
	GetPrefabBoundsRecursive(PrefabActor, Result);
	return Result;
}

void FPrefabTools::LoadStateFromPrefabAsset(APrefabActor* PrefabActor, const FPrefabLoadSettings& InSettings)
{
	if (!PrefabActor) {
		UE_LOG(LogPrefabTools, Error, TEXT("Invalid prefab actor reference"));
		return;
	}

	UPrefabricatorAsset* PrefabAsset = PrefabActor->GetPrefabAsset();
	if (!PrefabAsset) {
		UE_LOG(LogPrefabTools, Error, TEXT("Prefab asset is not assigned correctly"));
		return;
	}

	PrefabActor->GetRootComponent()->SetMobility(PrefabAsset->PrefabMobility);

	TArray<AActor*> ExistingActorPool;
	GetActorChildren(PrefabActor, ExistingActorPool);
	TMap<FGuid, AActor*> ActorByItemID;

	// Delete existing child actors that belong to this prefab
	for (AActor* ExistingActor : ExistingActorPool) {
		if (ExistingActor && ExistingActor->GetRootComponent()) {
			UPrefabricatorAssetUserData* PrefabUserData = ExistingActor->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
			if (PrefabUserData && PrefabUserData->PrefabActor == PrefabActor) {
				ActorByItemID.Add(PrefabUserData->ItemID, ExistingActor);
			}
		}
	}

	for (FPrefabricatorActorData& ActorItemData : PrefabAsset->ActorData) {
		UClass* ActorClass = LoadObject<UClass>(nullptr, *ActorItemData.ClassPath);
		if (!ActorClass) continue;

		UWorld* World = PrefabActor->GetWorld();
		AActor* ChildActor = nullptr;
		if (AActor** SearchResult = ActorByItemID.Find(ActorItemData.PrefabItemID)) {
			ChildActor = *SearchResult;
			FString ExistingClassName = ChildActor->GetClass()->GetPathName();
			FString RequiredClassName = ActorItemData.ClassPath;
			if (ExistingClassName == RequiredClassName) {
				// We can reuse this actor
				ExistingActorPool.Remove(ChildActor);
			}
		}

		if (!ChildActor) {
			FActorSpawnParameters SpawnParams;
			SpawnParams.OverrideLevel = PrefabActor->GetLevel();
			ChildActor = World->SpawnActor<AActor>(ActorClass, SpawnParams);
		}

		// Load the saved data into the actor
		LoadStateFromPrefabAsset(ChildActor, ActorItemData, InSettings);
		
		ParentActors(PrefabActor, ChildActor);
		AssignAssetUserData(ChildActor, ActorItemData.PrefabItemID, PrefabActor);

		// Set the transform
		FTransform WorldTransform = ActorItemData.RelativeTransform * PrefabActor->GetTransform();
		if (ChildActor->GetRootComponent()) {
			EComponentMobility::Type OldChildMobility = EComponentMobility::Movable;
			if (ChildActor->GetRootComponent()) {
				OldChildMobility = ChildActor->GetRootComponent()->Mobility;
			}
			ChildActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);
			ChildActor->SetActorTransform(WorldTransform);
			ChildActor->GetRootComponent()->SetMobility(OldChildMobility);
		}

		if (APrefabActor* ChildPrefab = Cast<APrefabActor>(ChildActor)) {
			if (InSettings.bRandomizeNestedSeed && InSettings.Random) {
				// This is a nested child prefab.  Randomize the seed of the child prefab
				ChildPrefab->Seed = FPrefabTools::GetRandomSeed(*InSettings.Random);
			}
			if (InSettings.bSynchronousBuild) {
				LoadStateFromPrefabAsset(ChildPrefab, InSettings);
			}
		}
	}

	// Destroy the unused actors from the pool
	for (AActor* UnusedActor : ExistingActorPool) {
		UnusedActor->Destroy();
	}

	PrefabActor->LastUpdateID = PrefabAsset->LastUpdateID;

	if (InSettings.bSynchronousBuild) {
		PrefabActor->HandleBuildComplete();
	}
}

#undef LOCTEXT_NAMESPACE


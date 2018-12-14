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
#include "Serialization/MemoryReader.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabTools, Log, All);

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

void FPrefabTools::UpdatePrefabThumbnail(UPrefabricatorAsset* PrefabAsset)
{
	if (!PrefabAsset) return;
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Service->UpdateThumbnail(PrefabAsset);
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

	PrefabActor->PrefabComponent->PrefabAsset = PrefabAsset;
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

	UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
	if (!PrefabAsset) {
		UE_LOG(LogPrefabTools, Error, TEXT("Prefab asset is not assigned correctly"));
		return;
	}

	PrefabAsset->PrefabMobility = PrefabActor->GetRootComponent()->Mobility;

	PrefabAsset->ActorData.Reset();

	TArray<AActor*> Children;
	GetActorChildren(PrefabActor, Children);

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

	bool HasDefaultValue(UProperty* Property, UObject* ObjToSerialize) {
		FString PropertyData, TemplatePropertyData;
		GetPropertyData(Property, ObjToSerialize, PropertyData);
		GetPropertyData(Property, ObjToSerialize->GetArchetype(), TemplatePropertyData);
		return (PropertyData == TemplatePropertyData);
	}

	bool ShouldSkipSerialization(UProperty* Property, UObject* ObjToSerialize, APrefabActor* PrefabActor) {
		if (UObjectProperty* ObjProperty = Cast<UObjectProperty>(Property)) {
			UObject* PropertyObjectValue = ObjProperty->GetObjectPropertyValue_InContainer(ObjToSerialize);
			if (ContainsOuterParent(PropertyObjectValue, ObjToSerialize) ||
				ContainsOuterParent(PropertyObjectValue, PrefabActor)) {
				//UE_LOG(LogPrefabTools, Log, TEXT("Skipping Property: %s"), *Property->GetName());
				return true;
			}
		}

		return false;
	}

	void DeserializeFields(UObject* InObjToDeserialize, const TArray<UPrefabricatorPropertyBase*>& InProperties) {
		TMap<FString, UPrefabricatorPropertyBase*> PropertiesByName;
		for (UPrefabricatorPropertyBase* Property : InProperties) {
			PropertiesByName.Add(Property->PropertyName, Property);
		}

		for (TFieldIterator<UProperty> PropertyIterator(InObjToDeserialize->GetClass()); PropertyIterator; ++PropertyIterator) {
			UProperty* Property = *PropertyIterator;
			if (!Property) continue;
			UPrefabricatorPropertyBase** SearchResult = PropertiesByName.Find(Property->GetName());
			if (!SearchResult) continue;
			UPrefabricatorPropertyBase* PrefabProperty = *SearchResult;

			if (UPrefabricatorAtomProperty* Atom = Cast<UPrefabricatorAtomProperty>(PrefabProperty)) {
				Property->ImportText(*Atom->ExportedValue, Property->ContainerPtrToValuePtr<void>(InObjToDeserialize), PPF_None, InObjToDeserialize);
			}
			else if (UPrefabricatorArrayProperty* Array = Cast<UPrefabricatorArrayProperty>(PrefabProperty)) {
				// ...
			}
			else if (UPrefabricatorSetProperty* Set = Cast<UPrefabricatorSetProperty>(PrefabProperty)) {
				// ...
			}
			else if (UPrefabricatorMapProperty* Map = Cast<UPrefabricatorMapProperty>(PrefabProperty)) {
				// ...
			}
		}
	}

	void SerializeFields(UObject* ObjToSerialize, APrefabActor* PrefabActor, TArray<UPrefabricatorPropertyBase*>& OutProperties) {
		UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
		if (!PrefabAsset) {
			return;
		}

		for (TFieldIterator<UProperty> PropertyIterator(ObjToSerialize->GetClass()); PropertyIterator; ++PropertyIterator) {
			UProperty* Property = *PropertyIterator;
			if (!Property) continue;
			if (FPrefabTools::ShouldIgnorePropertySerialization(Property->GetFName())) {
				continue;
			}

			UPrefabricatorPropertyBase* PrefabProperty = nullptr;
			FString PropertyName = Property->GetName();

			/*
			if (UArrayProperty* ArrayProperty = Cast<UArrayProperty>(Property)) {
				UPrefabricatorArrayProperty* ArrayPrefabProperty = NewObject<UPrefabricatorArrayProperty>(PrefabAsset);
				ArrayPrefabProperty->PropertyName = PropertyName;

				FScriptArrayHelper ArrayHelper(ArrayProperty, ArrayProperty->ContainerPtrToValuePtr<void>(ObjToSerialize));
				UProperty* ArrayItemProperty = ArrayProperty->Inner;

				for (int i = 0; i < ArrayHelper.Num(); i++) {
					void* ArrayItemData = ArrayHelper.GetRawPtr(i);
					FString ItemValue;
					ArrayItemProperty->ExportTextItem(ItemValue, ArrayItemData, nullptr, ObjToSerialize, PPF_None);
					ArrayPrefabProperty->ExportedValues.Add(ItemValue);
				}

				PrefabProperty = ArrayPrefabProperty;
			}
			else if (USetProperty* SetProperty = Cast<USetProperty>(Property)) {
				UPrefabricatorSetProperty* SetPrefabProperty = NewObject<UPrefabricatorSetProperty>(PrefabAsset);
				SetPrefabProperty->PropertyName = PropertyName;

				FScriptSetHelper SetHelper(SetProperty, SetProperty->ContainerPtrToValuePtr<void>(ObjToSerialize));
				UProperty* SetItemProperty = SetProperty->ElementProp;

				for (int i = 0; i < SetHelper.Num(); i++) {
					void* SetItemData = SetHelper.GetElementPtr(i);
					FString ItemValue;
					SetItemProperty->ExportTextItem(ItemValue, SetItemData, nullptr, ObjToSerialize, PPF_None);
					SetPrefabProperty->ExportedValues.Add(ItemValue);
				}

				PrefabProperty = SetPrefabProperty;
			}
			else if (UMapProperty* MapProperty = Cast<UMapProperty>(Property)) {
				UPrefabricatorMapProperty* MapPrefabProperty = NewObject<UPrefabricatorMapProperty>(PrefabAsset);
				MapPrefabProperty->PropertyName = PropertyName;

				FScriptMapHelper MapHelper(MapProperty, MapProperty->ContainerPtrToValuePtr<void>(ObjToSerialize));
				UProperty* MapKeyProperty = MapProperty->KeyProp;
				UProperty* MapValueProperty = MapProperty->ValueProp;
				
				for (int i = 0; i < MapHelper.Num(); i++) {
					void* KeyData = MapHelper.GetKeyPtr(i);
					void* ValueData = MapHelper.GetValuePtr(i);
					
					int32 EntryIdx = MapPrefabProperty->ExportedEntries.AddDefaulted();
					FPrefabricatorMapPropertyEntry& Entry = MapPrefabProperty->ExportedEntries[EntryIdx];
					MapKeyProperty->ExportTextItem(Entry.ExportedKey, KeyData, nullptr, ObjToSerialize, PPF_None);
					MapValueProperty->ExportTextItem(Entry.ExportedValue, ValueData, nullptr, ObjToSerialize, PPF_None);
				}

				PrefabProperty = MapPrefabProperty;
			}
			else 
			*/
			{
				if (HasDefaultValue(Property, ObjToSerialize) || ShouldSkipSerialization(Property, ObjToSerialize, PrefabActor)) {
					continue;
				}

				FString PropertyValue;
				Property->ExportTextItem(PropertyValue, Property->ContainerPtrToValuePtr<void>(ObjToSerialize), nullptr, ObjToSerialize, PPF_None);

				UPrefabricatorAtomProperty* AtomPrefabProperty = NewObject<UPrefabricatorAtomProperty>(PrefabAsset);
				AtomPrefabProperty->PropertyName = PropertyName;
				AtomPrefabProperty->ExportedValue = PropertyValue;

				PrefabProperty = AtomPrefabProperty;
				
			}

			if (PrefabProperty) {
				OutProperties.Add(PrefabProperty);
			}
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

	void DumpSerializedProperties(const TArray<UPrefabricatorPropertyBase*>& InProperties) {
		for (UPrefabricatorPropertyBase* Property : InProperties) {
			if (UPrefabricatorAtomProperty* Atom = Cast<UPrefabricatorAtomProperty>(Property)) {
				UE_LOG(LogPrefabTools, Log, TEXT("%s: %s"), *Atom->PropertyName, *Atom->ExportedValue);
			}
			else if (UPrefabricatorArrayProperty* Array = Cast<UPrefabricatorArrayProperty>(Property)) {
				UE_LOG(LogPrefabTools, Log, TEXT("%s: Array[%d]"), *Array->PropertyName, Array->ExportedValues.Num());
				for (int i = 0; i < Array->ExportedValues.Num(); i++) {
					UE_LOG(LogPrefabTools, Log, TEXT("\t%s"), *Array->ExportedValues[i]);
				}
			}
			else if (UPrefabricatorSetProperty* Set = Cast<UPrefabricatorSetProperty>(Property)) {
				UE_LOG(LogPrefabTools, Log, TEXT("%s: Set[%d]"), *Set->PropertyName, Set->ExportedValues.Num());
				for (int i = 0; i < Set->ExportedValues.Num(); i++) {
					UE_LOG(LogPrefabTools, Log, TEXT("\t%s"), *Set->ExportedValues[i]);
				}
			}
			else if (UPrefabricatorMapProperty* Map = Cast<UPrefabricatorMapProperty>(Property)) {
				UE_LOG(LogPrefabTools, Log, TEXT("%s: Map[%d]"), *Map->PropertyName, Map->ExportedEntries.Num());
				for (int i = 0; i < Map->ExportedEntries.Num(); i++) {
					UE_LOG(LogPrefabTools, Log, TEXT("\t%s <=> %s"), *Map->ExportedEntries[i].ExportedKey, *Map->ExportedEntries[i].ExportedValue);
				}
			}
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

	//DumpSerializedData(OutActorData);
}

void FPrefabTools::LoadStateFromPrefabAsset(AActor* InActor, APrefabActor* PrefabActor, const FPrefabricatorActorData& InActorData, const FPrefabLoadSettings& InSettings)
{
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
}

void FPrefabTools::GetActorChildren(AActor* InParent, TArray<AActor*>& OutChildren)
{
	InParent->GetAttachedActors(OutChildren);
}

namespace {
	void GetPrefabBoundsRecursive(AActor* InActor, FBox& OutBounds) {
		if (!InActor->IsA<APrefabActor>()) {
			OutBounds += InActor->GetComponentsBoundingBox(true);
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

	UPrefabricatorAsset* PrefabAsset = PrefabActor->PrefabComponent->PrefabAsset;
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
			ChildActor = World->SpawnActor<AActor>(ActorClass);
		}

		// Load the saved data into the actor
		LoadStateFromPrefabAsset(ChildActor, PrefabActor, ActorItemData, InSettings);
		
		ParentActors(PrefabActor, ChildActor);
		AssignAssetUserData(ChildActor, ActorItemData.PrefabItemID, PrefabActor);

		// Set the transform
		FTransform WorldTransform = ActorItemData.RelativeTransform * PrefabActor->GetTransform();
		ChildActor->SetActorTransform(WorldTransform);

		if (APrefabActor* ChildPrefab = Cast<APrefabActor>(ChildActor)) {
			FPrefabTools::LoadStateFromPrefabAsset(ChildPrefab, InSettings);
		}
	}

	// Destroy the unused actors from the pool
	for (AActor* UnusedActor : ExistingActorPool) {
		UnusedActor->Destroy();
	}

	PrefabActor->PrefabComponent->UpdateBounds();

	PrefabActor->LastUpdateID = PrefabAsset->LastUpdateID;
}


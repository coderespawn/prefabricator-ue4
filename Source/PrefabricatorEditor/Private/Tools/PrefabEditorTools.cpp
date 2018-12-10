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
#include "UnrealMemory.h"

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

///////////////////////////////////
/*
* Houses base functionality shared between CPFUO archivers (FCPFUOWriter/FCPFUOReader)
* Used to track whether tagged data is being processed (and whether we should be serializing it).
*/
struct FPrefabricatorBaseArchive
{
public:
	FPrefabricatorBaseArchive(bool bIncludeUntaggedDataIn)
		: bIncludeUntaggedData(bIncludeUntaggedDataIn)
		, TaggedDataScope(0)
	{}

	FPrefabricatorBaseArchive(const FPrefabricatorBaseArchive& DataSrc)
		: bIncludeUntaggedData(DataSrc.bIncludeUntaggedData)
		, TaggedDataScope(0)
	{}

protected:
	FORCEINLINE void OpenTaggedDataScope() { ++TaggedDataScope; }
	FORCEINLINE void CloseTaggedDataScope() { --TaggedDataScope; }

	FORCEINLINE bool IsSerializationEnabled()
	{
		return bIncludeUntaggedData || (TaggedDataScope > 0);
	}

	bool bIncludeUntaggedData;
private:
	int32 TaggedDataScope;
};

/* Serializes and stores property data from a specified 'source' object. Only stores data compatible with a target destination object. */
struct FPrefabricatorWriter : public FObjectWriter, public FPrefabricatorBaseArchive
{
public:
	FPrefabricatorWriter(UObject* SrcObject, TArray<uint8>& SavedPropertyData, const UEngine::FCopyPropertiesForUnrelatedObjectsParams& Params)
		: FObjectWriter(SavedPropertyData)
		// if the two objects don't share a common native base class, then they may have different
		// serialization methods, which is dangerous (the data is not guaranteed to be homogeneous)
		// in that case, we have to stick with tagged properties only
		, FPrefabricatorBaseArchive(true)
		, bSkipCompilerGeneratedDefaults(Params.bSkipCompilerGeneratedDefaults)
	{
		ArIgnoreArchetypeRef = true;
		ArNoDelta = !Params.bDoDelta;
		ArIgnoreClassRef = true;
		ArPortFlags |= Params.bCopyDeprecatedProperties ? PPF_UseDeprecatedProperties : PPF_None;

		SrcObject->Serialize(*this);
	}

	//~ Begin FArchive Interface
	virtual void Serialize(void* Data, int64 Num) override
	{
		if (IsSerializationEnabled())
		{
			FObjectWriter::Serialize(Data, Num);
		}
	}

	virtual void MarkScriptSerializationStart(const UObject* Object) override { OpenTaggedDataScope(); }
	virtual void MarkScriptSerializationEnd(const UObject* Object) override { CloseTaggedDataScope(); }

#if WITH_EDITOR
	virtual bool ShouldSkipProperty(const class UProperty* InProperty) const override
	{
		static FName BlueprintCompilerGeneratedDefaultsName(TEXT("BlueprintCompilerGeneratedDefaults"));
		return bSkipCompilerGeneratedDefaults && InProperty->HasMetaData(BlueprintCompilerGeneratedDefaultsName);
	}
#endif 
	//~ End FArchive Interface

private:

	bool bSkipCompilerGeneratedDefaults;
};

/* Responsible for applying the saved property data from a FCPFUOWriter to a specified object */
struct FPrefabricatorReader : public FObjectReader, public FPrefabricatorBaseArchive
{
public:
	FPrefabricatorReader(UObject* DstObject, TArray<uint8>& SavedPropertyData)
		: FObjectReader(SavedPropertyData)
		, FPrefabricatorBaseArchive(true)
	{
		ArIgnoreArchetypeRef = true;
		ArIgnoreClassRef = true;

#if USE_STABLE_LOCALIZATION_KEYS
		if (GIsEditor && !(ArPortFlags & (PPF_DuplicateVerbatim | PPF_DuplicateForPIE)))
		{
			SetLocalizationNamespace(TextNamespaceUtil::EnsurePackageNamespace(DstObject));
		}
#endif // USE_STABLE_LOCALIZATION_KEYS

		DstObject->Serialize(*this);
	}

	//~ Begin FArchive Interface
	virtual void Serialize(void* Data, int64 Num) override
	{
		if (IsSerializationEnabled())
		{
			FObjectReader::Serialize(Data, Num);
		}
	}

	virtual void MarkScriptSerializationStart(const UObject* Object) override { OpenTaggedDataScope(); }
	virtual void MarkScriptSerializationEnd(const UObject* Object) override { CloseTaggedDataScope(); }
	// ~End FArchive Interface
};

///////////////////////////////////

namespace {

	void GetPropertyData(UProperty* Property, UObject* Obj, TArray<uint8>& OutPropertyData) {
		if (Property) {
			int32 Size = Property->GetSize();
			uint8* SrcData = Property->ContainerPtrToValuePtr<uint8>(Obj);
			OutPropertyData.AddUninitialized(Size);
			FMemory::Memcpy(OutPropertyData.GetData(), SrcData, Size);
		}
	}

	void SerializeFields(UObject* Obj, TArray<FPrefabricatorFieldData>& OutFields) {
		for (TFieldIterator<UProperty> PropertyIterator(Obj->GetClass()); PropertyIterator; ++PropertyIterator) {
			UProperty* Property = *PropertyIterator;
			int32 Size = Property->GetSize();
			TArray<uint8> PropertyData, TemplatePropertyData;
			GetPropertyData(Property, Obj, PropertyData);
			GetPropertyData(Property, Obj->GetArchetype(), TemplatePropertyData);
			bool bDefaultValue = FMemory::Memcmp(PropertyData.GetData(), TemplatePropertyData.GetData(), Size) == 0;

			if (UObjectProperty* ObjProperty = Cast<UObjectProperty>(Property)) {
				UObject* PropertyObject = ObjProperty->GetObjectPropertyValue(Property->ContainerPtrToValuePtr<UObject*>(Obj));
				if (PropertyObject && (PropertyObject->IsA<AActor>() || PropertyObject->IsA<UActorComponent>())) {
					continue;
				}
			}

			if (bDefaultValue) {
				continue;
			}

			UE_LOG(LogPrefabEditorTools, Error, TEXT("Property: %s,  Size: %d"), *Property->GetName(), Size);

		}
	}
}

void FPrefabEditorTools::SaveStateToPrefabAsset(AActor* InActor, const FTransform& InversePrefabTransform, FPrefabricatorActorData& OutActorData)
{
	FTransform LocalTransform = InActor->GetTransform() * InversePrefabTransform;
	OutActorData.RelativeTransform = LocalTransform;
	OutActorData.ActorClass = InActor->GetClass();
	
	DumpPropertyList(InActor->GetRootComponent());
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
			GEditor->ParentActors(PrefabActor, ChildActor, NAME_None);
		}


		// Load the saved data into the actor
		{
			TMap<FName, FPrefabricatorComponentData*> ComponentDataByName;
			for (FPrefabricatorComponentData& ComponentData : ActorItemData.ComponentData) {
				if (!ComponentDataByName.Contains(ComponentData.ComponentName)) {
					ComponentDataByName.Add(ComponentData.ComponentName, &ComponentData);
				}
			}

			// TODO: ..
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


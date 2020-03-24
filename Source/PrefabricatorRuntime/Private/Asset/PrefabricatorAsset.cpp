//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Asset/PrefabricatorAsset.h"

#include "Prefab/PrefabTools.h"
#include "PrefabricatorSettings.h"
#include "Utils/PrefabricatorConstants.h"
#include "Utils/PrefabricatorService.h"
#include "Utils/PrefabricatorStats.h"

#include "GameFramework/Actor.h"
#include "Internationalization/Regex.h"
#include "Misc/PackageName.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabricatorAsset, Log, All);

UPrefabricatorAsset::UPrefabricatorAsset(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
}

UPrefabricatorAsset* UPrefabricatorAsset::GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig)
{
	return this;
}

FVector FPrefabricatorAssetUtils::FindPivot(const TArray<AActor*>& InActors)
{
	FVector Pivot = FVector::ZeroVector;
	if (InActors.Num() > 0) {
		float LowestZ = MAX_flt;
		FBox Bounds(EForceInit::ForceInit);
		for (AActor* Actor : InActors) {
			FBox ActorBounds = FPrefabTools::GetPrefabBounds(Actor, false);
			Bounds += ActorBounds;
		}

		switch (GetDefault< UPrefabricatorSettings>()->PivotPosition)
		{
		case EPrefabricatorPivotPosition::ExtremeLeft:
			Pivot = Bounds.GetCenter() - Bounds.GetExtent();
			break;
		case EPrefabricatorPivotPosition::ExtremeRight:
			Pivot = Bounds.GetCenter() + Bounds.GetExtent();
			break;
		case EPrefabricatorPivotPosition::Center:
			Pivot = Bounds.GetCenter();
			break;
		default:;
		}
		Pivot.Z = Bounds.Min.Z;
	}

	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Service.IsValid()) {
		Pivot = Service->SnapToGrid(Pivot);
	}

	return Pivot;
}

EComponentMobility::Type FPrefabricatorAssetUtils::FindMobility(const TArray<AActor*>& InActors)
{
	return EComponentMobility::Static;
	/*
	EComponentMobility::Type Mobility = EComponentMobility::Movable;
	for (AActor* Actor : InActors) {
		if (!Actor || !Actor->GetRootComponent()) {
			continue;
		}
		EComponentMobility::Type ActorMobility = Actor->GetRootComponent()->Mobility;
		if (Mobility == EComponentMobility::Movable && ActorMobility == EComponentMobility::Stationary) {
			Mobility = EComponentMobility::Stationary;
		}
		else if (ActorMobility == EComponentMobility::Static) {
			Mobility = EComponentMobility::Static;
		}
	}

	return Mobility;
	*/
}

///////////////////////////////////////// UPrefabricatorAssetCollection ///////////////////////////////////////// 

UPrefabricatorAssetCollection::UPrefabricatorAssetCollection(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer) {
	Version = (int32)EPrefabricatorCollectionAssetVersion::LatestVersion;
}

UPrefabricatorAsset* UPrefabricatorAssetCollection::GetPrefabAsset(const FPrefabAssetSelectionConfig& InConfig)
{
	if (Prefabs.Num() == 0) return nullptr;

	float TotalWeight = 0.0f;
	for (const FPrefabricatorAssetCollectionItem& Item : Prefabs) {
		TotalWeight += FMath::Max(0.0f, Item.Weight);
	}

	FRandomStream Random;
	Random.Initialize(InConfig.Seed);

	TSoftObjectPtr<UPrefabricatorAsset> PrefabAssetPtr;

	if (TotalWeight == 0) {
		// Return a random value from the list
		int32 Index = Random.RandRange(0, Prefabs.Num() - 1);
		PrefabAssetPtr = Prefabs[Index].PrefabAsset;
	}
	else {
		float SelectionValue = Random.FRandRange(0, TotalWeight);
		float StartRange = 0.0f;
		bool bFound = false;
		for (const FPrefabricatorAssetCollectionItem& Item : Prefabs) {
			float EndRange = StartRange + Item.Weight;
			if (SelectionValue >= StartRange && SelectionValue < EndRange) {
				PrefabAssetPtr = Item.PrefabAsset;
				bFound = true;
				break;
			}
			StartRange = EndRange;
		}
		if (!bFound) {
			PrefabAssetPtr = Prefabs.Last().PrefabAsset;
		}
	}
	return PrefabAssetPtr.LoadSynchronous();
}

void UPrefabricatorEventListener::PostSpawn_Implementation(APrefabActor* Prefab)
{

}

void UPrefabricatorProperty::SaveReferencedAssetValues()
{
	AssetSoftReferenceMappings.Reset();

	const FString SoftReferenceSearchPattern = "([A-Za-z0-9_]+)'(.*?)'";

	const FRegexPattern Pattern(*SoftReferenceSearchPattern);
	FRegexMatcher Matcher(Pattern, *ExportedValue);

	while (Matcher.FindNext()) {
		FString FullPath = Matcher.GetCaptureGroup(0);
		FString ClassName = Matcher.GetCaptureGroup(1);
		FString ObjectPath = Matcher.GetCaptureGroup(2);
		if (ClassName == "PrefabricatorAssetUserData") {
			continue;
		}
		bool bUseQuotes = false;
		if (ObjectPath.Len() >= 2 && ObjectPath.StartsWith("\"") && ObjectPath.EndsWith("\"")) {
			ObjectPath = ObjectPath.Mid(1, ObjectPath.Len() - 2);
			bUseQuotes = true;
		}

		/*
		int32 StartIdx = Matcher.GetMatchBeginning();
		int32 EndIdx = Matcher.GetMatchEnding();
		FString AssetPath = ExportedValue.Mid(StartIdx, EndIdx - StartIdx + 1);
		if (AssetPath.StartsWith("PrefabricatorAssetUserData")) {		// TODO: Get this name from the static class
			continue;
		}
		*/

		FSoftObjectPath SoftPath(ObjectPath);

		FPrefabricatorPropertyAssetMapping Mapping;
		Mapping.AssetReference = SoftPath;
		//if (Mapping.AssetReference.IsValid()) 
		{
			//FString ObjectPathString;
			//FPackageName::ParseExportTextPath(AssetPath, &Mapping.AssetClassName, &ObjectPathString);
			Mapping.AssetClassName = ClassName;
			Mapping.AssetObjectPath = *ObjectPath;
			Mapping.bUseQuotes = bUseQuotes;
			AssetSoftReferenceMappings.Add(Mapping);
			UE_LOG(LogPrefabricatorAsset, Log, TEXT("######>>> Found Asset: [%s][%s] | %s"), *Mapping.AssetClassName, *Mapping.AssetObjectPath.ToString(), *Mapping.AssetReference.GetAssetPathName().ToString());
		}
	}
}

void UPrefabricatorProperty::LoadReferencedAssetValues()
{
	SCOPE_CYCLE_COUNTER(STAT_LoadReferencedAssetValues);
	bool bModified = false;
	for (FPrefabricatorPropertyAssetMapping& Mapping : AssetSoftReferenceMappings) {
		// Check if the name has changed
		//if (!Mapping.AssetReference.IsValid()) {
		//	continue;
		//}

		FName ReferencedPath;
		{
			//SCOPE_CYCLE_COUNTER(STAT_LoadReferencedAssetValues_GetAssetPathName);
			ReferencedPath = Mapping.AssetReference.GetAssetPathName();
			if (ReferencedPath.ToString().IsEmpty()) {
				continue;
			}

			if (ReferencedPath == Mapping.AssetObjectPath) {
				// No change in the exported text path and the referenced path
				continue;
			}
		}

		// The object path has changed.  Update it and mark as modified
		FString ReplaceFrom, ReplaceTo;
		{
			SCOPE_CYCLE_COUNTER(STAT_LoadReferencedAssetValues_Replacements1);
			if (Mapping.bUseQuotes) {
				ReplaceFrom = FString::Printf(TEXT("%s\'\"%s\"\'"), *Mapping.AssetClassName, *Mapping.AssetObjectPath.ToString());
				ReplaceTo = FString::Printf(TEXT("%s\'\"%s\"\'"), *Mapping.AssetClassName, *ReferencedPath.ToString());
			}
			else {
				ReplaceFrom = FString::Printf(TEXT("%s\'%s\'"), *Mapping.AssetClassName, *Mapping.AssetObjectPath.ToString());
				ReplaceTo = FString::Printf(TEXT("%s\'%s\'"), *Mapping.AssetClassName, *ReferencedPath.ToString());
			}
		}

		{
			SCOPE_CYCLE_COUNTER(STAT_LoadReferencedAssetValues_Replacements2);
			ExportedValue = ExportedValue.Replace(*ReplaceFrom, *ReplaceTo);
		}
		Mapping.AssetObjectPath = ReferencedPath;

		bModified = true;
	}

	if (bModified) {
		SCOPE_CYCLE_COUNTER(STAT_LoadReferencedAssetValues_Modify);
		Modify();
	}
}


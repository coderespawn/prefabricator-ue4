//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

DECLARE_STATS_GROUP(TEXT("Prefabricator"), STATGROUP_Prefabricator, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Randomize - LoadPrefab"), STAT_Randomize_LoadPrefab, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("Randomize - GetChildActor"), STAT_Randomize_GetChildActor, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadState - DeserializeFields [Actor]"), STAT_LoadStateFromPrefabAsset_DeserializeFieldsActor, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadState - Begin Transaction"), STAT_LoadStateFromPrefabAsset_BeginTransaction, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadState - End Transaction"), STAT_LoadStateFromPrefabAsset_EndTransaction, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadState - DesFields [Components]"), STAT_LoadStateFromPrefabAsset_DeserializeFieldsComponents, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadState - Unreg Comp"), STAT_LoadStateFromPrefabAsset_UnregisterComponent, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadState - Reg Comp"), STAT_LoadStateFromPrefabAsset_RegisterComponent, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("DeserializeFields - BuildMap"), STAT_DeserializeFields_BuildMap, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate"), STAT_DeserializeFields_Iterate, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> LoadValue"), STAT_DeserializeFields_Iterate_LoadValue, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue"), STAT_DeserializeFields_Iterate_SetValue, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadRefVal"), STAT_LoadReferencedAssetValues, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - GetAssetPathName"), STAT_LoadReferencedAssetValues_GetAssetPathName, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 1"), STAT_LoadReferencedAssetValues_Replacements1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 2"), STAT_LoadReferencedAssetValues_Replacements2, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Modify"), STAT_LoadReferencedAssetValues_Modify, STATGROUP_Prefabricator);


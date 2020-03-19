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
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 1"), STAT_DeserializeFields_Iterate_SetValue1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 2"), STAT_DeserializeFields_Iterate_SetValue2, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 3"), STAT_DeserializeFields_Iterate_SetValue3, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 4"), STAT_DeserializeFields_Iterate_SetValue4, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 5"), STAT_DeserializeFields_Iterate_SetValue5, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 6"), STAT_DeserializeFields_Iterate_SetValue6, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 7"), STAT_DeserializeFields_Iterate_SetValue7, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 8"), STAT_DeserializeFields_Iterate_SetValue8, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 9"), STAT_DeserializeFields_Iterate_SetValue9, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 10"), STAT_DeserializeFields_Iterate_SetValue10, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 11"), STAT_DeserializeFields_Iterate_SetValue11, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 12"), STAT_DeserializeFields_Iterate_SetValue12, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 13"), STAT_DeserializeFields_Iterate_SetValue13, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 14"), STAT_DeserializeFields_Iterate_SetValue14, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 15"), STAT_DeserializeFields_Iterate_SetValue15, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue 16"), STAT_DeserializeFields_Iterate_SetValue16, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadRefVal"), STAT_LoadReferencedAssetValues, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - GetAssetPathName"), STAT_LoadReferencedAssetValues_GetAssetPathName, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 1"), STAT_LoadReferencedAssetValues_Replacements1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 2"), STAT_LoadReferencedAssetValues_Replacements2, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Modify"), STAT_LoadReferencedAssetValues_Modify, STATGROUP_Prefabricator);


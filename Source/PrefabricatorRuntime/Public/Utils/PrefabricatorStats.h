//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"

DECLARE_STATS_GROUP(TEXT("Prefabricator"), STATGROUP_Prefabricator, STATCAT_Advanced);
DECLARE_CYCLE_STAT(TEXT("Randomize - LoadPrefab"), STAT_Randomize_LoadPrefab, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("Randomize - GetChildActor"), STAT_Randomize_GetChildActor, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset [ALL]"), STAT_LoadStateFromPrefabAsset, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset - Actor Loop"), STAT_LoadStateFromPrefabAsset_ActorLoop, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset 1"), STAT_LoadStateFromPrefabAsset1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset 2"), STAT_LoadStateFromPrefabAsset2, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset 3"), STAT_LoadStateFromPrefabAsset3, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset 4"), STAT_LoadStateFromPrefabAsset4, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadStateFromPrefabAsset 5"), STAT_LoadStateFromPrefabAsset5, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("ParentActors - [ALL]"), STAT_ParentActors, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("ParentActors - 1"), STAT_ParentActors1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("ParentActors - 2"), STAT_ParentActors2, STATGROUP_Prefabricator);


DECLARE_CYCLE_STAT(TEXT("LoadActorState [ALL]"), STAT_LoadActorState, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - Begin Transaction"), STAT_LoadActorState_BeginTransaction, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - End Transaction"), STAT_LoadActorState_EndTransaction, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - DeserializeFields [Actor]"), STAT_LoadActorState_DeserializeFieldsActor, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - DesFields [Components]"), STAT_LoadActorState_DeserializeFieldsComponents, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - Unreg Comp"), STAT_LoadActorState_UnregisterComponent, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadActorState - Reg Comp"), STAT_LoadActorState_RegisterComponent, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("DeserializeFields - BuildMap"), STAT_DeserializeFields_BuildMap, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate"), STAT_DeserializeFields_Iterate, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> LoadValue"), STAT_DeserializeFields_Iterate_LoadValue, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("DeserializeFields - Iterate -> SetValue"), STAT_DeserializeFields_Iterate_SetValue, STATGROUP_Prefabricator);

DECLARE_CYCLE_STAT(TEXT("LoadRefVal"), STAT_LoadReferencedAssetValues, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - GetAssetPathName"), STAT_LoadReferencedAssetValues_GetAssetPathName, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 1"), STAT_LoadReferencedAssetValues_Replacements1, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Replacements 2"), STAT_LoadReferencedAssetValues_Replacements2, STATGROUP_Prefabricator);
DECLARE_CYCLE_STAT(TEXT("LoadRefVal - Modify"), STAT_LoadReferencedAssetValues_Modify, STATGROUP_Prefabricator);


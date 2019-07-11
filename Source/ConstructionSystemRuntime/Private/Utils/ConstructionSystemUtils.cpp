//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemUtils.h"
#include "Engine/CollisionProfile.h"
#include "ConstructionSystemDefs.h"

ECollisionChannel FConstructionSystemUtils::FindPrefabSnapChannel()
{
	// UCollisionProfile::ReturnContainerIndexFromChannelName is not exported with ENGINE_API so a less optimized way is used below
	UCollisionProfile* CollisionSettings = UCollisionProfile::Get();
	UEnum* Enum = StaticEnum<ECollisionChannel>();
	int32 NumEnums = Enum->NumEnums();
	for (int i = 0; i < NumEnums; i++) {
		FName ChannelName = CollisionSettings->ReturnChannelNameFromContainerIndex(i);
		if (ChannelName == FConstructionSystemConstants::PrefabSnapChannelName) {
			return (ECollisionChannel)Enum->GetValueByIndex(i);
		}
	}
	return ECollisionChannel::ECC_WorldStatic;
}

//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/ProjectSettingsUpdater.h"

#include "Engine/CollisionProfile.h"

void FProjectSettingsUpdater::IsConstructionSystemPhysicsSetup()
{
	TArray<TSharedPtr<FName>> RegisteredProfileNames;
	UCollisionProfile* CollisionSettings = UCollisionProfile::Get();
	
}

void FProjectSettingsUpdater::AddConstructionSystemCollisionSettings()
{

	FCollisionResponseTemplate Profile;
	Profile.Name = "PrefabSnap";
	Profile.CollisionEnabled = ECollisionEnabled::QueryOnly;
	Profile.ObjectType = ECollisionChannel::ECC_WorldStatic;
	Profile.ObjectTypeName = "WorldStatic";
	Profile.HelpMessage = "Prefab Snap Collision Presets";
	Profile.bCanModify = true;

	Profile.CustomResponses.Add(FResponseChannel("PrefabSnap", ECollisionResponse::ECR_Overlap));
#define IGNORE_OBJ_TYPE(Name) Profile.CustomResponses.Add(FResponseChannel(#Name, ECollisionResponse::ECR_Ignore))
	IGNORE_OBJ_TYPE(WorldStatic);
	IGNORE_OBJ_TYPE(WorldDynamic);
	IGNORE_OBJ_TYPE(Pawn);
	IGNORE_OBJ_TYPE(Visibility);
	IGNORE_OBJ_TYPE(PhysicsBody);
	IGNORE_OBJ_TYPE(Vehicle);
	IGNORE_OBJ_TYPE(Destructible);
#undef IGNORE_OBJ_TYPE


	UCollisionProfile* CollisionSettings = UCollisionProfile::Get();
	
}


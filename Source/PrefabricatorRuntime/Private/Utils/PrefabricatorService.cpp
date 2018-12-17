//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorService.h"

#include "Engine/EngineTypes.h"

/////////////////////////// FPrefabricatorService /////////////////////////// 

TSharedPtr<IPrefabricatorService> FPrefabricatorService::Instance = nullptr;

TSharedPtr<IPrefabricatorService> FPrefabricatorService::Get()
{
	return Instance;
}

void FPrefabricatorService::Set(TSharedPtr<IPrefabricatorService> InInstance)
{
	Instance = InInstance;
}


/////////////////////////// FPrefabricatorRuntimeService /////////////////////////// 
void FPrefabricatorRuntimeService::ParentActors(AActor* ParentActor, AActor* ChildActor)
{
	ChildActor->AttachToActor(ParentActor, FAttachmentTransformRules(EAttachmentRule::KeepWorld, true));
}

void FPrefabricatorRuntimeService::SelectPrefabActor(AActor* PrefabActor)
{
	// Not supported in runtime builds (and not necessary)
}

void FPrefabricatorRuntimeService::GetSelectedActors(TArray<AActor*>& OutActors)
{
	// Not supported in runtime builds (and not necessary)
}

int FPrefabricatorRuntimeService::GetNumSelectedActors()
{
	// Not supported in runtime builds (and not necessary)
	return 0;
}

UPrefabricatorAsset* FPrefabricatorRuntimeService::CreatePrefabAsset()
{
	// Not supported in runtime builds (and not necessary)
	return nullptr;
}

//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/PrefabricatorService.h"

#include "Engine/EngineTypes.h"
#include "Engine/World.h"

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


/////////////////////////// IPrefabricatorService /////////////////////////// 
AActor* IPrefabricatorService::SpawnActor(TSubclassOf<AActor> InClass, const FTransform& InTransform, ULevel* InLevel, AActor* InTemplate)
{
	if (!InClass || !InLevel) {
		return nullptr;
	}

	TArray<AActor*> AttachedToTemplate;
	if (InTemplate) {
		InTemplate->GetAttachedActors(AttachedToTemplate);
		for (AActor* TemplateChild : AttachedToTemplate) {
			TemplateChild->DetachFromActor(FDetachmentTransformRules(EDetachmentRule::KeepWorld, false));
		}
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.OverrideLevel = InLevel;
	SpawnParams.Template = InTemplate;
	UWorld* World = InLevel->GetWorld();
	AActor* Actor = World->SpawnActor<AActor>(InClass, InTransform, SpawnParams);
	TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
	if (Actor && InTemplate && Service.IsValid()) {
		// Attach the template children back
		for (AActor* TemplateChild : AttachedToTemplate) {
			Service->ParentActors(InTemplate, TemplateChild);
		}
	}
	return Actor;
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


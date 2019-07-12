//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/ConstructionSystemSnap.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"


///////////////////////////// UPrefabricatorBoxSnapComponent ///////////////////////////// 
UPrefabricatorConstructionSnapComponent::UPrefabricatorConstructionSnapComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	BoxExtent = FVector(100, 100, 100);
	SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SetCollisionProfileName("PrefabSnap");
}

void UPrefabricatorConstructionSnapComponent::OnRegister()
{
	Super::OnRegister();

}

///////////////////////////// AConstructionSnapPoint ///////////////////////////// 
APrefabricatorConstructionSnap::APrefabricatorConstructionSnap(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	bCanBeDamaged = false;
	bRelevantForLevelBounds = false;

	ConstructionSnapComponent = CreateDefaultSubobject<UPrefabricatorConstructionSnapComponent>(TEXT("SnapComponent"));
	ConstructionSnapComponent->Mobility = EComponentMobility::Static;
	ConstructionSnapComponent->SetGenerateOverlapEvents(true);
	RootComponent = ConstructionSnapComponent;

}


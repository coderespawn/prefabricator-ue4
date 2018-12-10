//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "PrefabActor.h"
#include "PrefabComponent.h"
#include "Engine/PointLight.h"
#include "Components/BillboardComponent.h"


APrefabActor::APrefabActor(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PrefabComponent = ObjectInitializer.CreateDefaultSubobject<UPrefabComponent>(this, "PrefabComponent");
	RootComponent = PrefabComponent;

	Sprite = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, "Sprite");
	Sprite->SetupAttachment(RootComponent);
}


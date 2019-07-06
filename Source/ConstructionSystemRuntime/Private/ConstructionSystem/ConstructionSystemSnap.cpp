//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystem/ConstructionSystemSnap.h"
#include "Components/SphereComponent.h"
#include "Components/ArrowComponent.h"

AConstructionSnapPoint::AConstructionSnapPoint(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer) 
{
	bCanBeDamaged = false;

	OverlapQueryComponent = CreateDefaultSubobject<USphereComponent>(TEXT("OverlapQuery"));
	OverlapQueryComponent->Mobility = EComponentMobility::Static;
	OverlapQueryComponent->SetGenerateOverlapEvents(true);
	OverlapQueryComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapQueryComponent->SetSphereRadius(OverlapQueryRadius);

	RootComponent = OverlapQueryComponent;

#if WITH_EDITOR
	ArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	ArrowComponent->Mobility = EComponentMobility::Static;
	ArrowComponent->SetHiddenInGame(true);
	ArrowComponent->SetupAttachment(RootComponent);
#endif // WITH_EDITOR
}

#if WITH_EDITOR


void AConstructionSnapPoint::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	if (e.Property) {
		FName PropertyName = e.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(AConstructionSnapPoint, OverlapQueryRadius)) {
			OverlapQueryComponent->SetSphereRadius(OverlapQueryRadius);
		}
	}
}
#endif // WITH_EDITOR

void AConstructionSnapPoint::ConnectToSnapPoint(AConstructionSnapPoint* A, AConstructionSnapPoint* B)
{
	check(!A->IsConnected() && !B->IsConnected());
	A->ConnectedSnapPoint = B;
	B->ConnectedSnapPoint = A;
}

bool AConstructionSnapPoint::IsSnapped(AConstructionSnapPoint* A, AConstructionSnapPoint* B)
{
	return (A && B && A->ConnectedSnapPoint == B && B->ConnectedSnapPoint == A);
}

void AConstructionSnapPoint::DisconnectSnapPoints(AConstructionSnapPoint* A, AConstructionSnapPoint* B)
{
	check(IsSnapped(A, B));
	A->ConnectedSnapPoint = nullptr;
	B->ConnectedSnapPoint = nullptr;
}

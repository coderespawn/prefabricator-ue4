//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/SphereComponent.h"
#include "ConstructionSystemSnap.generated.h"

class USphereComponent;
class UArrowComponent;

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API AConstructionSnapPoint : public AActor
{
	GENERATED_UCLASS_BODY()

private:
	UPROPERTY()
	USphereComponent* OverlapQueryComponent;

	UPROPERTY()
	TWeakObjectPtr<AConstructionSnapPoint> ConnectedSnapPoint;

	UFUNCTION()
	bool IsConnected() const { return ConnectedSnapPoint.IsValid(); }

	static void ConnectToSnapPoint(AConstructionSnapPoint* A, AConstructionSnapPoint* B);
	static bool IsSnapped(AConstructionSnapPoint* A, AConstructionSnapPoint* B);
	static void DisconnectSnapPoints(AConstructionSnapPoint* A, AConstructionSnapPoint* B);

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& e) override;

	UPROPERTY(EditAnywhere, Category = "ConstructionSystem")
	float OverlapQueryRadius = 60;

	UPROPERTY()
	UArrowComponent* ArrowComponent;
#endif // WITH_EDITOR

};

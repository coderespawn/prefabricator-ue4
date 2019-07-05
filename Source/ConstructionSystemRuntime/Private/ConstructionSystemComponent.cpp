//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "DrawDebugHelpers.h"
#include "PrefabActor.h"
#include "PrefabComponent.h"
#include "PrefabTools.h"
#include "Components/PrimitiveComponent.h"
#include "PrefabricatorAsset.h"
#include "PrefabricatorFunctionLibrary.h"


UConstructionSystemComponent::UConstructionSystemComponent()
	: CursorGhostActor(nullptr)
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UConstructionSystemComponent::EnableConstructionSystem()
{
	TransitionCameraTo(ConstructionCameraActor, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
	bConstructionSystemEnabled = true;
}

void UConstructionSystemComponent::DisableConstructionSystem()
{
	TransitionCameraTo(GetOwner(), ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
	bConstructionSystemEnabled = false;
}

void UConstructionSystemComponent::SetActivePrefab(UPrefabricatorAssetInterface* InActivePrefabAsset)
{
	ActivePrefabAsset = InActivePrefabAsset;
	RecreateCursorGhost();
}

void UConstructionSystemComponent::ConstructAtCursor()
{
	if (bConstructionSystemEnabled) {
		UWorld* World = GetWorld();
		if (!World) {
			return;
		}

		if (World && CursorGhostActor && ActivePrefabAsset) {
			FTransform Transform = CursorGhostActor->GetActorTransform();
			APrefabActor* SpawnedPrefab = UPrefabricatorBlueprintLibrary::SpawnPrefab(World, ActivePrefabAsset, Transform, 0);

			if (bRandomizedPlacement) {
				UPrefabricatorBlueprintLibrary::RandomizePrefab(SpawnedPrefab, RandomStream);
			}
		}
	}
}

void UConstructionSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bConstructionSystemEnabled) {
		UpdateConstructionSystem();
	}
}

void UConstructionSystemComponent::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	Super::DestroyComponent();
	if (CursorGhostActor) {
		CursorGhostActor->Destroy();
		CursorGhostActor = nullptr;
	}
}

APlayerController* UConstructionSystemComponent::GetPlayerController()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	return Owner ? Owner->GetController<APlayerController>() : nullptr; 
}

void UConstructionSystemComponent::TransitionCameraTo(AActor* InViewTarget, float InBlendTime, float InBlendExp)
{
	if (InViewTarget) {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController) {
			PlayerController->SetViewTargetWithBlend(InViewTarget, InBlendTime, VTBlend_Cubic, InBlendExp);
		}
	}
}

void UConstructionSystemComponent::UpdateConstructionSystem()
{
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	APlayerController* PlayerController = GetPlayerController();
	if (PlayerController) {
		FVector ViewLocation;
		FRotator ViewRotation;
		PlayerController->GetPlayerViewPoint(ViewLocation, ViewRotation);

		FVector StartLocation = ViewLocation;
		FVector CameraDirection = ViewRotation.RotateVector(FVector::ForwardVector);
		FVector EndLocation = StartLocation + CameraDirection * TraceDistance;

		FHitResult Hit;
		FCollisionQueryParams QueryParams = FCollisionQueryParams::DefaultQueryParam;
		QueryParams.AddIgnoredActor(GetOwner());
		if (CursorGhostActor) {
			FPrefabTools::IterateChildrenRecursive(CursorGhostActor, [&QueryParams](AActor* ChildCursorActor) {
				QueryParams.AddIgnoredActor(ChildCursorActor);
			});
		}
		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;

		if (World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams, ResponseParams)) {
			if (CursorGhostActor) {
				CursorGhostActor->SetActorLocation(Hit.ImpactPoint);
			}
			DrawDebugPoint(World, Hit.ImpactPoint, 20, FColor::Red);
		}
	}
}

void UConstructionSystemComponent::RecreateCursorGhost()
{
	if (CursorGhostActor) {
		CursorGhostActor->Destroy();
		CursorGhostActor = nullptr;
	}

	if (ActivePrefabAsset) {
		CursorGhostActor = GetWorld()->SpawnActor<APrefabActor>();
		CursorGhostActor->PrefabComponent->PrefabAssetInterface = ActivePrefabAsset;
		CursorGhostActor->LoadPrefab();
		CursorGhostActor->GetRootComponent()->SetMobility(EComponentMobility::Movable);

		FPrefabTools::IterateChildrenRecursive(CursorGhostActor, [this](AActor* ChildActor) {
			for (UActorComponent* Component : ChildActor->GetComponents()) {
				if (UPrimitiveComponent* Primitive = Cast<UPrimitiveComponent>(Component)) {
					// Disable collision
					Primitive->SetCollisionEnabled(ECollisionEnabled::NoCollision);

					// Set cursor material
					if (CursorMaterial) {
						int32 NumMaterials = Primitive->GetNumMaterials();
						for (int ElementIndex = 0; ElementIndex < NumMaterials; ElementIndex++) {
							Primitive->SetMaterial(ElementIndex, CursorMaterial);
						}
					}
				}
			}
		});

		bool bCursorHidden = !bConstructionSystemEnabled;
		CursorGhostActor->SetActorHiddenInGame(bCursorHidden);
	}
}

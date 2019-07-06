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
#include "ConstructionSystemCursor.h"


UConstructionSystemComponent::UConstructionSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	Cursor = CreateDefaultSubobject<UConstructionSystemCursor>("Cursor");
}

void UConstructionSystemComponent::EnableConstructionSystem()
{
	Cursor->RecreateCursor(GetWorld(), ActivePrefabAsset, CursorMaterial);
	TransitionCameraTo(ConstructionCameraActor, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
	bConstructionSystemEnabled = true;
}

void UConstructionSystemComponent::DisableConstructionSystem()
{
	Cursor->DestroyCursor();
	TransitionCameraTo(GetOwner(), ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
	bConstructionSystemEnabled = false;
}

void UConstructionSystemComponent::SetActivePrefab(UPrefabricatorAssetInterface* InActivePrefabAsset)
{
	ActivePrefabAsset = InActivePrefabAsset;
	Cursor->RecreateCursor(GetWorld(), InActivePrefabAsset, CursorMaterial);
}

void UConstructionSystemComponent::ConstructAtCursor()
{
	if (bConstructionSystemEnabled) {
		UWorld* World = GetWorld();
		if (!World) {
			return;
		}

		if (World && ActivePrefabAsset) {
			FTransform Transform;
			if (Cursor->GetCursorTransform(Transform)) {
				APrefabActor* SpawnedPrefab = GetWorld()->SpawnActor<APrefabActor>(APrefabActor::StaticClass(), Transform);
				SpawnedPrefab->PrefabComponent->PrefabAssetInterface = ActivePrefabAsset;

				FRandomStream RandomStream(Cursor->GetCursorSeed());
				UPrefabricatorBlueprintLibrary::RandomizePrefab(SpawnedPrefab, RandomStream);
			}
		}
	}
}

void UConstructionSystemComponent::CursorMoveNext()
{
	Cursor->IncrementSeed();
	Cursor->RecreateCursor(GetWorld(), ActivePrefabAsset, CursorMaterial);
}

void UConstructionSystemComponent::CursorMovePrev()
{
	Cursor->DecrementSeed();
	Cursor->RecreateCursor(GetWorld(), ActivePrefabAsset, CursorMaterial);
}

void UConstructionSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bConstructionSystemEnabled) {
		HandleUpdate();
	}
}

void UConstructionSystemComponent::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	Super::DestroyComponent();
	Cursor->DestroyCursor();
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

void UConstructionSystemComponent::HandleUpdate()
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
		if (Cursor->GetCursorGhostActor()) {
			FPrefabTools::IterateChildrenRecursive(Cursor->GetCursorGhostActor(), [&QueryParams](AActor* ChildCursorActor) {
				QueryParams.AddIgnoredActor(ChildCursorActor);
			});
		}
		FCollisionResponseParams ResponseParams = FCollisionResponseParams::DefaultResponseParam;

		if (World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_WorldStatic, QueryParams, ResponseParams)) {
			Cursor->SetTransform(Hit.ImpactPoint, Hit.Normal);
		}

		// Draw debug info
		DrawDebugPoint(World, Hit.ImpactPoint, 20, FColor::Red);
	}
}

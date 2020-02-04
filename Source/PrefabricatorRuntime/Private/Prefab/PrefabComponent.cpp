//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/PrefabComponent.h"

#include "Asset/PrefabricatorAsset.h"
#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabTools.h"
#include "Utils/PrefabricatorService.h"

#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabComponent, Log, All);

UPrefabComponent::UPrefabComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
	if (!IsRunningCommandlet()) {
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> PrefabSpriteObject(TEXT("/Prefabricator/PrefabTool/Sprites/Icon_48"));
		EditorSpriteTexture = PrefabSpriteObject.Get();
	}
#endif // WITH_EDITORONLY_DATA
}

void UPrefabComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		SpriteComponent->SetSprite(EditorSpriteTexture);
		SpriteComponent->SetRelativeScale3D(FVector(2.0f));
		SpriteComponent->SpriteInfo.Category = TEXT("Prefab");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("PrefabComponent", "Prefab", "Prefab");
		SpriteComponent->Mobility = EComponentMobility::Static;
	}
#endif //WITH_EDITORONLY_DATA
}

FBoxSphereBounds UPrefabComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	APrefabActor* PrefabActor = Cast<APrefabActor>(GetOwner());
	if (PrefabActor) {
		FBox BoundsBox = FPrefabTools::GetPrefabBounds(PrefabActor);
		return FBoxSphereBounds(BoundsBox);
	}
	else {
		return FBoxSphereBounds(EForceInit::ForceInitToZero);
	}
}


#if WITH_EDITOR
void UPrefabComponent::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);

	if (e.Property) {
		FName PropertyName = e.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UPrefabComponent, PrefabAssetInterface)) {
			APrefabActor* PrefabActor = Cast<APrefabActor>(GetOwner());
			if (PrefabActor) {
				if (PrefabActor->IsPrefabOutdated()) {
					PrefabActor->LoadPrefab();
				}

				// Update the property view so the new UI takes effect
				TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
				if (Service.IsValid()) {
					Service->SetDetailsViewObject(PrefabActor);
				}
			}
		}
	}

}
#endif // WITH_EDITOR


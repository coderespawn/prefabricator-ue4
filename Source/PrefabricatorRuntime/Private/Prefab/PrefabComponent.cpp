//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/PrefabComponent.h"

#include "Prefab/PrefabActor.h"

#include "Components/BillboardComponent.h"
#include "Engine/Texture2D.h"
#include "UObject/ConstructorHelpers.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabComponent, Log, All);

UPrefabComponent::UPrefabComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

	bVisualizeComponent = true;

#if WITH_EDITORONLY_DATA
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
		SpriteComponent->RelativeScale3D = FVector(2.0f);
		SpriteComponent->SpriteInfo.Category = TEXT("Prefab");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("PrefabComponent", "Prefab", "Prefab");
		SpriteComponent->Mobility = EComponentMobility::Static;
	}

#endif //WITH_EDITORONLY_DATA
}


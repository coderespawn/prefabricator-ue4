//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/Random/PrefabSeedLinker.h"

#include "Components/BillboardComponent.h"
#include "UObject/ConstructorHelpers.h"

/////////////////////////////////// UPrefabSeedLinkerComponent ///////////////////////////////////

UPrefabSeedLinkerComponent::UPrefabSeedLinkerComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

#if WITH_EDITORONLY_DATA
	bVisualizeComponent = true;
	if (!IsRunningCommandlet()) {
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> PrefabSpriteObject(TEXT("/Prefabricator/PrefabTool/Sprites/Icon_48_linker"));
		EditorSpriteTexture = PrefabSpriteObject.Get();
	}
#endif // WITH_EDITORONLY_DATA
}

void UPrefabSeedLinkerComponent::OnRegister()
{
	Super::OnRegister();

#if WITH_EDITORONLY_DATA
	if (SpriteComponent)
	{
		SpriteComponent->SetSprite(EditorSpriteTexture);
		SpriteComponent->SetRelativeScale3D(FVector(2.0f));
		SpriteComponent->SpriteInfo.Category = TEXT("PrefabLinker");
		SpriteComponent->SpriteInfo.DisplayName = NSLOCTEXT("PrefabLinkerComponent", "PrefabLinker", "PrefabLinker");
		SpriteComponent->Mobility = EComponentMobility::Static;
	}
#endif //WITH_EDITORONLY_DATA
}

/////////////////////////////////// APrefabSeedLinker ///////////////////////////////////

APrefabSeedLinker::APrefabSeedLinker(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer) 
{
	SeedLinkerComponent = ObjectInitializer.CreateDefaultSubobject<UPrefabSeedLinkerComponent>(this, "SeedLinker");
	RootComponent = SeedLinkerComponent;
}

#if WITH_EDITOR
FName APrefabSeedLinker::GetCustomIconName() const
{
	static const FName PrefabIconName("ClassIcon.PrefabSeedLinkerActor");
	return PrefabIconName;
}
#endif // WITH_EDITOR


//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "GameFramework/Actor.h"
#include "PrefabSeedLinker.generated.h"

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API UPrefabSeedLinkerComponent : public USceneComponent {
	GENERATED_UCLASS_BODY()
public:
	virtual void OnRegister() override;

private:
#if WITH_EDITORONLY_DATA
	UPROPERTY()
	UTexture2D* EditorSpriteTexture;
#endif // WITH_EDITORONLY_DATA
};


UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class PREFABRICATORRUNTIME_API APrefabSeedLinker : public AActor {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	TArray<TWeakObjectPtr<class APrefabActor>> LinkedActors;

	UPROPERTY()
	UPrefabSeedLinkerComponent* SeedLinkerComponent;

public:

#if WITH_EDITOR
	virtual FName GetCustomIconName() const override;
#endif // WITH_EDITOR

};


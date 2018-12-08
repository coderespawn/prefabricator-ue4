//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrefabActor.generated.h"

/** A Dungeon Theme asset lets you design the look and feel of you dungeon with an intuitive graph based approach */
UCLASS(Blueprintable, ConversionRoot, ComponentWrapperClass)
class PREFABRICATORRUNTIME_API APrefabActor : public AActor {
	GENERATED_UCLASS_BODY()
public:
	UPROPERTY(Category = Prefabricator, VisibleAnywhere, BlueprintReadOnly)
	class UPrefabComponent* PrefabComponent;

};

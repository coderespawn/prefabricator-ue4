//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PrefabDebugActor.generated.h"

UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API APrefabDebugActor : public AActor {
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Debug")
	AActor* Actor;

	UPROPERTY()
	TArray<uint8> ActorData;

public:
	void SaveActorData();
	void LoadActorData();

};



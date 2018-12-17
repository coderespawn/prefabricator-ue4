//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "PrefabRandomizerActor.generated.h"



UCLASS(Blueprintable)
class PREFABRICATORRUNTIME_API APrefabRandomizer : public AActor {
	GENERATED_UCLASS_BODY()

public:
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable)
	void Randomize(const FRandomStream& InRandom);

public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	bool bRandomizeOnBeginPlay = true;

public:
	UPROPERTY(EditAnywhere, Category = "Prefabricator")
	int32 SeedOffset = 0;

};

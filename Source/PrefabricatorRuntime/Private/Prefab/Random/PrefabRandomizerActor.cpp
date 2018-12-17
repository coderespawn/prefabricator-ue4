//$ Copyright 2015-18, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/Random/PrefabRandomizerActor.h"
#include "RandomStream.h"
#include "GameFramework/Actor.h"
#include "Components/SceneComponent.h"
#include "Components/BillboardComponent.h"
#include "ConstructorHelpers.h"
#include "PrefabActor.h"
#include "EngineUtils.h"
#include "PrefabSeedLinker.h"


APrefabRandomizer::APrefabRandomizer(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	USceneComponent* SceneRoot = ObjectInitializer.CreateDefaultSubobject<USceneComponent>(this, "SceneRoot");
	RootComponent = SceneRoot;

#if WITH_EDITORONLY_DATA
	UBillboardComponent* SpriteComponent = ObjectInitializer.CreateDefaultSubobject<UBillboardComponent>(this, "Sprite");
	SpriteComponent->SetupAttachment(RootComponent);

	if (!IsRunningCommandlet()) {
		ConstructorHelpers::FObjectFinderOptional<UTexture2D> PrefabSpriteObject(TEXT("/Prefabricator/PrefabTool/Sprites/Icon_48_randomizer"));
		SpriteComponent->SetSprite(PrefabSpriteObject.Get());
	}
#endif // WITH_EDITORONLY_DATA
}

void APrefabRandomizer::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomizeOnBeginPlay) {
		FRandomStream Random;
		int32 Seed = FMath::Abs(SeedOffset + (int32)GetTypeHash(GetActorLocation()));
		Random.Initialize(Seed);
		Randomize(Random);
	}
}

namespace {
	template<typename T>
	void GetActorsInLevel(ULevel* Level, TArray<T*>& OutResult) {
		for (TActorIterator<T> It(Level->GetWorld()); It; ++It) {
			T* Actor = *It;
			if (Actor->GetLevel() == Level) {
				OutResult.Add(Actor);
			}
		}
	}

	template<typename T>
	void SanitizeArray(TArray<TWeakObjectPtr<T>>& InOutArray) {
		for (int i = 0; i < InOutArray.Num(); i++) {
			if (!InOutArray[i].IsValid()) {
				InOutArray.RemoveAt(i);
				i--;
			}
		}
	}
}

void APrefabRandomizer::Randomize(const FRandomStream& InRandom)
{
	// Grab all the actors in the level
	ULevel* CurrentLevel = GetLevel();
	TArray<APrefabActor*> PrefabsInLevel;
	GetActorsInLevel(CurrentLevel, PrefabsInLevel);

	for (APrefabActor* PrefabActor : PrefabsInLevel) {
		PrefabActor->RandomizeSeed(InRandom, false);
	}

	TArray<APrefabSeedLinker*> SeedLinkersInLevel;
	GetActorsInLevel(CurrentLevel, SeedLinkersInLevel);
	for (APrefabSeedLinker* SeedLinker : SeedLinkersInLevel) {
		SanitizeArray(SeedLinker->LinkedActors);
		if (SeedLinker->LinkedActors.Num() > 1) {
			int32 LinkedSeeds = SeedLinker->LinkedActors[0]->Seed;
			for (int i = 1; i < SeedLinker->LinkedActors.Num(); i++) {
				SeedLinker->LinkedActors[i]->Seed = LinkedSeeds;
			}
		}
	}

	for (APrefabActor* PrefabActor : PrefabsInLevel) {
		if (PrefabActor->IsPrefabOutdated()) {
			PrefabActor->LoadPrefab();
		}
	}
}

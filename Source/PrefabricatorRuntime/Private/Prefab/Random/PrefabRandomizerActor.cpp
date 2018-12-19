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
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

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

void APrefabRandomizer::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (BuildQueue.IsValid()) {
		BuildQueue->Tick();
	}
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
	TArray<APrefabActor*> AllPrefabsInLevel;
	GetActorsInLevel(CurrentLevel, AllPrefabsInLevel);

	// Build only the top level prefabs
	TArray<APrefabActor*> TopLevelPrefabs = AllPrefabsInLevel.FilterByPredicate([](APrefabActor* InPrefab) -> bool {
		AActor* Parent = InPrefab->GetAttachParentActor();
		bool bChildOfAnotherPrefab = Parent && Parent->IsA<APrefabActor>();
		return !bChildOfAnotherPrefab;
	});

	for (APrefabActor* PrefabActor : TopLevelPrefabs) {
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


	BuildQueue = MakeShareable(new FPrefabBuildQueue(MaxBuildTimePerFrame));
	for (APrefabActor* TopLevelPrefab : TopLevelPrefabs) {
		FPrefabBuildQueueItem BuildItem;
		BuildItem.bRandomizeNestedSeed = true;
		BuildItem.Random = InRandom;
		BuildItem.Prefab = TopLevelPrefab;
		BuildQueue->Enqueue(BuildItem);
	}

	BuildQueue->Tick();
}

#if WITH_EDITOR
FName APrefabRandomizer::GetCustomIconName() const
{
	static const FName PrefabIconName("ClassIcon.PrefabRandomizerActor");
	return PrefabIconName;
}
#endif // WITH_EDITOR


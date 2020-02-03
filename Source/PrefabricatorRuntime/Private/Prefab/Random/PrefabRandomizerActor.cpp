//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/Random/PrefabRandomizerActor.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/Random/PrefabSeedLinker.h"

#include "Components/BillboardComponent.h"
#include "Components/SceneComponent.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"
#include "Math/RandomStream.h"
#include "UObject/ConstructorHelpers.h"

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

	if (BuildSystem.IsValid()) {
		BuildSystem->Tick();
		int32 NumRemaining = BuildSystem->GetNumPendingCommands();
		if (NumRemaining == 0) {
			OnRandomizationComplete.Broadcast();
			BuildSystem = nullptr;
		}
	}
}

void APrefabRandomizer::BeginPlay()
{
	Super::BeginPlay();

	if (bRandomizeOnBeginPlay) {
		int32 Seed = FMath::Abs(SeedOffset + (int32)GetTypeHash(GetActorLocation()));
		Randomize(Seed);
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

void APrefabRandomizer::Randomize(int32 InSeed)
{
	Random.Initialize(InSeed);

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
		PrefabActor->RandomizeSeed(Random, false);
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

	BuildSystem = MakeShareable(new FPrefabBuildSystem(MaxBuildTimePerFrame));
	for (APrefabActor* TopLevelPrefab : TopLevelPrefabs) {
		FPrefabBuildSystemCommandPtr BuildCommand;
		if (bFastSyncBuild) {
			BuildCommand = MakeShareable(new FPrefabBuildSystemCommand_BuildPrefabSync(TopLevelPrefab, true, &Random));
		}
		else {
			BuildCommand = MakeShareable(new FPrefabBuildSystemCommand_BuildPrefab(TopLevelPrefab, true, &Random));
		}

		BuildSystem->PushCommand(BuildCommand);
	}

	BuildSystem->Tick();
}

#if WITH_EDITOR
FName APrefabRandomizer::GetCustomIconName() const
{
	static const FName PrefabIconName("ClassIcon.PrefabRandomizerActor");
	return PrefabIconName;
}
#endif // WITH_EDITOR


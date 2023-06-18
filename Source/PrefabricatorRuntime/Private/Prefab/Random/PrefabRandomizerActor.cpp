//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/Random/PrefabRandomizerActor.h"

#include "Prefab/PrefabActor.h"
#include "Prefab/PrefabTools.h"
#include "Prefab/Random/PrefabSeedLinker.h"
#include "Utils/PrefabricatorService.h"

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

			// Run GC in the editor
			TSharedPtr<IPrefabricatorService> Service = FPrefabricatorService::Get();
			if (Service.IsValid()) {
				Service->RunGC();
			}
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
		if (Level) {
			for (AActor* Actor : Level->Actors) {
				if (T* CastActor = Cast<T>(Actor)) {
					OutResult.Add(CastActor);
				}
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

	const bool bRandomizeEverythingInLevel = (ActorsToRandomize.Num() == 0); 
	TArray<APrefabActor*> TargetActors;
	ULevel* CurrentLevel = GetLevel();
	if (bRandomizeEverythingInLevel) {
		// Grab all the actors in the level
		GetActorsInLevel(CurrentLevel, TargetActors);
	}
	else {
		TargetActors = ActorsToRandomize;
	}

	if (TargetActors.Num() == 0) return;
	
	// Build only the top level prefabs
	TArray<APrefabActor*> TopLevelPrefabs = TargetActors.FilterByPredicate([](APrefabActor* InPrefab) -> bool {
		AActor* Parent = InPrefab->GetAttachParentActor();
		const bool bChildOfAnotherPrefab = Parent && Parent->IsA<APrefabActor>();
		return !bChildOfAnotherPrefab;
	});

	for (APrefabActor* PrefabActor : TopLevelPrefabs) {
		PrefabActor->RandomizeSeed(Random, false);
	}

	TArray<APrefabSeedLinker*> SeedLinkersInLevel;
	GetActorsInLevel(CurrentLevel, SeedLinkersInLevel);
	for (APrefabSeedLinker* SeedLinker : SeedLinkersInLevel) {
		SanitizeArray(SeedLinker->LinkedActors);

		// If we are randomizing only selected actors, check if the seed linker links to one of them
		bool bValidSeedLinker;
		if (!bRandomizeEverythingInLevel) {
			bValidSeedLinker = false;
			for (TWeakObjectPtr<APrefabActor> LinkedActor : SeedLinker->LinkedActors) {
				if (!LinkedActor.IsValid()) continue;
				if (TargetActors.Contains(LinkedActor)) {
					bValidSeedLinker = true;
					break;
				}
			}
		}
		else {
			bValidSeedLinker = true;
		}
		
		if (bValidSeedLinker && SeedLinker->LinkedActors.Num() > 1) {
			const int32 LinkedSeeds = SeedLinker->LinkedActors[0]->Seed;
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


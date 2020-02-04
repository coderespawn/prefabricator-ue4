//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Prefab/PrefabActor.h"

#include "Asset/PrefabricatorAsset.h"
#include "Asset/PrefabricatorAssetUserData.h"
#include "Prefab/PrefabComponent.h"
#include "Prefab/PrefabTools.h"
#include "Utils/PrefabricatorStats.h"

#include "Components/BillboardComponent.h"
#include "Engine/PointLight.h"

DEFINE_LOG_CATEGORY_STATIC(LogPrefabActor, Log, All);


APrefabActor::APrefabActor(const FObjectInitializer& ObjectInitializer) 
	: Super(ObjectInitializer)
{
	PrefabComponent = ObjectInitializer.CreateDefaultSubobject<UPrefabComponent>(this, "PrefabComponent");
	RootComponent = PrefabComponent;
}

namespace {
	void DestroyAttachedActorsRecursive(AActor* ActorToDestroy, TSet<AActor*>& Visited) {
		if (!ActorToDestroy || !ActorToDestroy->GetRootComponent()) return;

		if (Visited.Contains(ActorToDestroy)) return;
		Visited.Add(ActorToDestroy);

		UPrefabricatorAssetUserData* PrefabUserData = ActorToDestroy->GetRootComponent()->GetAssetUserData<UPrefabricatorAssetUserData>();
		if (!PrefabUserData) return;

		UWorld* World = ActorToDestroy->GetWorld();
		if (!World) return;

		TArray<AActor*> AttachedActors;
		ActorToDestroy->GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors) {
			DestroyAttachedActorsRecursive(AttachedActor, Visited);
		}
		ActorToDestroy->Destroy();
	}
}

void APrefabActor::Destroyed()
{
	Super::Destroyed();

	// Destroy all attached actors
	{
		TSet<AActor*> Visited;
		TArray<AActor*> AttachedActors;
		GetAttachedActors(AttachedActors);
		for (AActor* AttachedActor : AttachedActors) {
			DestroyAttachedActorsRecursive(AttachedActor, Visited);
		}
	}
}

void APrefabActor::PostLoad()
{
	Super::PostLoad();

}

void APrefabActor::PostActorCreated()
{
	Super::PostActorCreated();

}

#if WITH_EDITOR
void APrefabActor::PostEditChangeProperty(struct FPropertyChangedEvent& e)
{
	Super::PostEditChangeProperty(e);


}

void APrefabActor::PostDuplicate(EDuplicateMode::Type DuplicateMode)
{
	Super::PostDuplicate(DuplicateMode);

	if (DuplicateMode == EDuplicateMode::Normal) {
		FRandomStream Random;
		Random.Initialize(FMath::Rand());
		RandomizeSeed(Random);

		FPrefabLoadSettings LoadSettings;
		LoadSettings.bRandomizeNestedSeed = true;
		LoadSettings.Random = &Random;
		FPrefabLoadStatePtr LoadState = MakeShareable(new FPrefabLoadState);
		FPrefabTools::LoadStateFromPrefabAsset(this, LoadSettings, LoadState);
	}
}

FName APrefabActor::GetCustomIconName() const
{
	static const FName PrefabIconName("ClassIcon.PrefabActor");
	return PrefabIconName;
}

#endif // WITH_EDITOR

void APrefabActor::LoadPrefab()
{
	FPrefabLoadStatePtr LoadState = MakeShareable(new FPrefabLoadState);
	FPrefabTools::LoadStateFromPrefabAsset(this, FPrefabLoadSettings(), LoadState);
}

void APrefabActor::SavePrefab()
{
	FPrefabTools::SaveStateToPrefabAsset(this);
}

bool APrefabActor::IsPrefabOutdated()
{
	UPrefabricatorAsset* PrefabAsset = GetPrefabAsset();
	if (!PrefabAsset) {
		return false;
	}

	return PrefabAsset->LastUpdateID != LastUpdateID;
}

UPrefabricatorAsset* APrefabActor::GetPrefabAsset()
{
	FPrefabAssetSelectionConfig SelectionConfig;
	SelectionConfig.Seed = Seed;
	UPrefabricatorAssetInterface* PrefabAssetInterface = PrefabComponent->PrefabAssetInterface.LoadSynchronous();
	return PrefabAssetInterface ? PrefabAssetInterface->GetPrefabAsset(SelectionConfig) : nullptr;
}

void APrefabActor::RandomizeSeed(const FRandomStream& InRandom, bool bRecursive)
{
	Seed = FPrefabTools::GetRandomSeed(InRandom);
	if (bRecursive) {
		TArray<AActor*> AttachedChildren;
		GetAttachedActors(AttachedChildren);
		for (AActor* AttachedActor : AttachedChildren) {
			if (APrefabActor* ChildPrefab = Cast<APrefabActor>(AttachedActor)) {
				ChildPrefab->RandomizeSeed(InRandom, bRecursive);
			}
		}
	}
}

void APrefabActor::HandleBuildComplete()
{
	UPrefabricatorAssetInterface* PrefabAssetInterface = PrefabComponent->PrefabAssetInterface.LoadSynchronous();
	if (PrefabAssetInterface && PrefabAssetInterface->EventListener) {
		UPrefabricatorEventListener* EventListenerInstance = NewObject<UPrefabricatorEventListener>(GetTransientPackage(), PrefabAssetInterface->EventListener, NAME_None, RF_Transient);
		if (EventListenerInstance) {
			EventListenerInstance->PostSpawn(this);
		}
	}
}

////////////////////////////////// FPrefabBuildSystem //////////////////////////////////
FPrefabBuildSystem::FPrefabBuildSystem(double InTimePerFrame)
	: TimePerFrame(InTimePerFrame)
{
}

void FPrefabBuildSystem::Tick()
{
	double StartTime = FPlatformTime::Seconds();
	
	
	while (BuildStack.Num() > 0) {
		FPrefabBuildSystemCommandPtr Item = BuildStack.Pop();
		Item->Execute(*this);

		if (TimePerFrame > 0) {
			double ElapsedTime = FPlatformTime::Seconds() - StartTime;
			if (ElapsedTime >= TimePerFrame) {
				break;
			}
		}
	}
}

void FPrefabBuildSystem::Reset()
{
	BuildStack.Reset();
}

void FPrefabBuildSystem::PushCommand(FPrefabBuildSystemCommandPtr InCommand)
{
	BuildStack.Push(InCommand);
}

FPrefabBuildSystemCommand_BuildPrefab::FPrefabBuildSystemCommand_BuildPrefab(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom, FPrefabLoadStatePtr InLoadState)
	: Prefab(InPrefab)
	, bRandomizeNestedSeed(bInRandomizeNestedSeed)
	, Random(InRandom)
	, LoadState(InLoadState)
{
}

void FPrefabBuildSystemCommand_BuildPrefab::Execute(FPrefabBuildSystem& BuildSystem)
{
	if (Prefab.IsValid()) {
		FPrefabLoadSettings LoadSettings;
		LoadSettings.bRandomizeNestedSeed = bRandomizeNestedSeed;
		LoadSettings.Random = Random;

		// Nested prefabs will be recursively build on the stack over multiple frames
		LoadSettings.bSynchronousBuild = false;

		{
			SCOPE_CYCLE_COUNTER(STAT_Randomize_LoadPrefab);
			FPrefabTools::LoadStateFromPrefabAsset(Prefab.Get(), LoadSettings, LoadState);
		}

		// Push a build complete notification request. Since this is a stack, it will execute after all the children are processed below
		FPrefabBuildSystemCommandPtr ChildBuildCommand = MakeShareable(new FPrefabBuildSystemCommand_NotifyBuildComplete(Prefab));
		BuildSystem.PushCommand(ChildBuildCommand);
	}

	// Add the child prefabs to the stack
	TArray<AActor*> ChildActors;
	{
		SCOPE_CYCLE_COUNTER(STAT_Randomize_GetChildActor);
		Prefab->GetAttachedActors(ChildActors);
	}
	for (AActor* ChildActor : ChildActors) {
		if (APrefabActor* ChildPrefab = Cast<APrefabActor>(ChildActor)) {
			FPrefabBuildSystemCommandPtr ChildBuildCommand = MakeShareable(new FPrefabBuildSystemCommand_BuildPrefab(ChildPrefab, bRandomizeNestedSeed, Random, LoadState));
			BuildSystem.PushCommand(ChildBuildCommand);
		}
	}
}

/////////////////////////////////////

FPrefabBuildSystemCommand_BuildPrefabSync::FPrefabBuildSystemCommand_BuildPrefabSync(TWeakObjectPtr<APrefabActor> InPrefab, bool bInRandomizeNestedSeed, FRandomStream* InRandom, FPrefabLoadStatePtr InLoadState)
	: Prefab(InPrefab)
	, bRandomizeNestedSeed(bInRandomizeNestedSeed)
	, Random(InRandom) 
	, LoadState(InLoadState)
{
}

void FPrefabBuildSystemCommand_BuildPrefabSync::Execute(FPrefabBuildSystem& BuildSystem)
{
	double StartTime = FPlatformTime::Seconds();
	if (Prefab.IsValid()) {
		Prefab->RandomizeSeed(*Random);

		FPrefabLoadSettings LoadSettings;
		LoadSettings.bRandomizeNestedSeed = true;
		LoadSettings.Random = Random;
		FPrefabTools::LoadStateFromPrefabAsset(Prefab.Get(), LoadSettings, LoadState);
	}
	double EndTime = FPlatformTime::Seconds();
	UE_LOG(LogTemp, Warning, TEXT("Exec Time: %fs"), (EndTime - StartTime));
}

/////////////////////////////////////

FPrefabBuildSystemCommand_NotifyBuildComplete::FPrefabBuildSystemCommand_NotifyBuildComplete(TWeakObjectPtr<APrefabActor> InPrefab)
	: Prefab(InPrefab)
{
}

void FPrefabBuildSystemCommand_NotifyBuildComplete::Execute(FPrefabBuildSystem& BuildSystem)
{
	if (Prefab.IsValid()) {
		// TODO: Execute Post spawn script
		Prefab->HandleBuildComplete();
	}
}


/////////////////////////////////////

AReplicablePrefabActor::AReplicablePrefabActor(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void AReplicablePrefabActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

}

void AReplicablePrefabActor::BeginPlay()
{
	if (GetLocalRole() == ROLE_Authority)
	{
		bReplicates = false;
		SetRemoteRoleForBackwardsCompat(ROLE_SimulatedProxy);
		SetReplicates(true);
	}

	Super::BeginPlay();
}


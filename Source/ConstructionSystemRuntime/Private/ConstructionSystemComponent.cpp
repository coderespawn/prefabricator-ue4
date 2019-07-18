//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "ConstructionSystemTool.h"
#include "ConstructionSystemBuildTool.h"
#include "Engine/ActorChannel.h"
#include "UnrealNetwork.h"
#include "ConstructionSystemRemoveTool.h"
#include "UserWidget.h"
#include "ConstructionSystemUI.h"

DEFINE_LOG_CATEGORY_STATIC(LogConstructionSystem, Log, All);

UConstructionSystemComponent::UConstructionSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bReplicates = true;
}


void UConstructionSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bConstructionSystemEnabled) {
		HandleUpdate();
	}

	APawn* Owner = Cast<APawn>(GetOwner());
	if (!bInputBound && Owner->InputEnabled()) {
		BindInput();
		CreateBuildMenu();
		bInputBound = true;
	}
}

void UConstructionSystemComponent::DestroyComponent(bool bPromoteChildren /*= false*/)
{
	Super::DestroyComponent();
	if (ActiveTool) {
		ActiveTool->DestroyTool(this);
		ActiveTool = nullptr;
	}
}

void UConstructionSystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

bool UConstructionSystemComponent::ReplicateSubobjects(class UActorChannel *Channel, class FOutBunch *Bunch, FReplicationFlags *RepFlags)
{
	bool bResult = Super::ReplicateSubobjects(Channel, Bunch, RepFlags);

	if (ActiveTool)
	{
		bResult |= Channel->ReplicateSubobject(ActiveTool, *Bunch, *RepFlags);
	}

	return bResult;
}

void UConstructionSystemComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UConstructionSystemComponent, ActiveTool);
}

void UConstructionSystemComponent::EnableConstructionSystem()
{
	TransitionCameraTo(ConstructionCameraActor, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);

	if (!ActiveTool) {
		// Create a default tool object
		CreateTool(UConstructionSystemBuildTool::StaticClass());
	}
	ActiveTool->OnToolEnable(this);

	bConstructionSystemEnabled = true;
}

void UConstructionSystemComponent::DisableConstructionSystem()
{
	if (ActiveTool) {
		ActiveTool->OnToolDisable(this);
	}

	TransitionCameraTo(GetOwner(), ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
	bConstructionSystemEnabled = false;
}

void UConstructionSystemComponent::CreateTool(TSubclassOf<UConstructionSystemTool> InToolClass)
{
	if (ActiveTool) {
		ActiveTool->DestroyTool(this);
		ActiveTool = nullptr;
	}

	if (InToolClass) {
		ActiveTool = NewObject<UConstructionSystemTool>(this, InToolClass);
		ActiveTool->InitializeTool(this);
		if (bConstructionSystemEnabled) {
			ActiveTool->OnToolEnable(this);
		}
	}
}

void UConstructionSystemComponent::CreateTool_Build()
{
	CreateTool(UConstructionSystemBuildTool::StaticClass());
}

void UConstructionSystemComponent::CreateTool_Remove()
{
	CreateTool(UConstructionSystemRemoveTool::StaticClass());
}

APlayerController* UConstructionSystemComponent::GetPlayerController()
{
	APawn* Owner = Cast<APawn>(GetOwner());
	return Owner ? Owner->GetController<APlayerController>() : nullptr; 
}

void UConstructionSystemComponent::TransitionCameraTo(AActor* InViewTarget, float InBlendTime, float InBlendExp)
{
	if (InViewTarget) {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController) {
			PlayerController->SetViewTargetWithBlend(InViewTarget, InBlendTime, VTBlend_Cubic, InBlendExp);
		}
	}
}

void UConstructionSystemComponent::HandleUpdate()
{
	UWorld* World = GetWorld();
	if (!World) {
		return;
	}

	if (ActiveTool) {
		ActiveTool->Update(this);
	}
}

void UConstructionSystemComponent::BindInput()
{
	APawn* Pawn = Cast<APawn>(GetOwner());
	if (Pawn && Pawn->InputComponent) {
		UInputComponent* Input = Pawn->InputComponent;
		Input->BindAction("CSModeToggle", IE_Pressed, this, &UConstructionSystemComponent::ToggleConstructionSystem);
		Input->BindAction("CSModeToolBuild", IE_Pressed, this, &UConstructionSystemComponent::CreateTool_Build);
		Input->BindAction("CSModeToolRemove", IE_Pressed, this, &UConstructionSystemComponent::CreateTool_Remove);
		Input->BindAction("CSToggleBuildUI", IE_Pressed, this, &UConstructionSystemComponent::ToggleBuildUI);
	}
}

void UConstructionSystemComponent::CreateBuildMenu()
{
	if (BuildMenuUI) {
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController) {
			BuildMenuUIInstance = CreateWidget(PlayerController, BuildMenuUI);
			if (BuildMenuUIInstance) {
				bool bImplementsInterface = BuildMenuUI->ImplementsInterface(UConstructionSystemBuildUI::StaticClass());
				if (!bImplementsInterface) {
					UE_LOG(LogConstructionSystem, Error, TEXT("Build Menu UI does not implement the interface ConstructionSystemBuildUI"));
				}
				IConstructionSystemBuildUI::Execute_SetConstructionSystem(BuildMenuUIInstance, this);
				IConstructionSystemBuildUI::Execute_SetUIAsset(BuildMenuUIInstance, BuildMenuData);
			}
		}
	}
}

void UConstructionSystemComponent::ShowBuildMenu()
{
	if (BuildMenuUIInstance) {
		BuildMenuUIInstance->AddToViewport();
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController) {
			PlayerController->SetInputMode(FInputModeUIOnly());
			PlayerController->bShowMouseCursor = true;
		}
	}
}

void UConstructionSystemComponent::HideBuildMenu()
{
	if (BuildMenuUIInstance) {
		BuildMenuUIInstance->RemoveFromParent();
		APlayerController* PlayerController = GetPlayerController();
		if (PlayerController) {
			PlayerController->SetInputMode(FInputModeGameOnly());
			PlayerController->bShowMouseCursor = false;
		}
	}
}

void UConstructionSystemComponent::ToggleBuildUI()
{

	if (BuildMenuUIInstance) {
		if (BuildMenuUIInstance->IsInViewport()) {
			HideBuildMenu();
		}
		else {
			ShowBuildMenu();
		}
	}
}

void UConstructionSystemComponent::ToggleConstructionSystem()
{
	if (bConstructionSystemEnabled) {
		DisableConstructionSystem();
	}
	else {
		EnableConstructionSystem();
	}
}

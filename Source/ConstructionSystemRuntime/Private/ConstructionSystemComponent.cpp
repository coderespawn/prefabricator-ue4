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
	bConstructionSystemEnabled = false;
}

void UConstructionSystemComponent::BeginPlay()
{
	Super::BeginPlay();

	bInputBound = false;
	CreateTools();
}

void UConstructionSystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	DestroyTools();
}

void UConstructionSystemComponent::SetActiveTool(EConstructionSystemToolType InToolType)
{
	UConstructionSystemTool* OldTool = nullptr;
	UConstructionSystemTool* NewTool = nullptr;
	{
		EConstructionSystemToolType OldToolType = ActiveToolType;
		EConstructionSystemToolType NewToolType = InToolType;

		UConstructionSystemTool** OldToolPtr = Tools.Find(OldToolType);
		UConstructionSystemTool** NewToolPtr = Tools.Find(NewToolType);

		OldTool = OldToolPtr ? *OldToolPtr : nullptr;
		NewTool = NewToolPtr ? *NewToolPtr : nullptr;
	}
	ActiveToolType = InToolType;
	if (!NewTool) {
		UE_LOG(LogConstructionSystem, Error, TEXT("Unsupported tool type. Cannot set active tool"));
	}

	if (bConstructionSystemEnabled) {
		if (OldTool) {
			OldTool->OnToolDisable(this);
		}

		if (NewTool) {
			NewTool->OnToolEnable(this);
		}
	}
}

UConstructionSystemTool* UConstructionSystemComponent::GetActiveTool()
{
	return GetTool(ActiveToolType);
}

UConstructionSystemTool* UConstructionSystemComponent::GetTool(EConstructionSystemToolType InToolType)
{
	UConstructionSystemTool** ToolPtr = Tools.Find(InToolType);
	return ToolPtr ? *ToolPtr : nullptr;
}

void UConstructionSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bConstructionSystemEnabled) {
		HandleUpdate();
	}

	APlayerController* PC = GetPlayerController();
	if (!bInputBound && PC && PC->InputEnabled()) {
		BindInput(PC->InputComponent);
		CreateBuildMenu();
		bInputBound = true;
	}
}

void UConstructionSystemComponent::EnableConstructionSystem(EConstructionSystemToolType InToolType)
{
	if (!bConstructionSystemEnabled) {
		TransitionCameraTo(ConstructionCameraActor, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
		bConstructionSystemEnabled = true;
	}
	SetActiveTool(InToolType);
}

void UConstructionSystemComponent::DisableConstructionSystem()
{
	if (bConstructionSystemEnabled) {
		UConstructionSystemTool* ActiveTool = GetActiveTool();
		if (ActiveTool) {
			ActiveTool->OnToolDisable(this);
		}

		APawn* ViewTarget = GetControlledPawn();
		TransitionCameraTo(ViewTarget, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);
		bConstructionSystemEnabled = false;
	}
}

void UConstructionSystemComponent::CreateTools()
{
	_CreateTool(EConstructionSystemToolType::BuildTool, UConstructionSystemBuildTool::StaticClass());
	_CreateTool(EConstructionSystemToolType::RemoveTool, UConstructionSystemRemoveTool::StaticClass());
}

void UConstructionSystemComponent::DestroyTools()
{
	for (auto& Entry : Tools) {
		UConstructionSystemTool* Tool = Entry.Value;
		if (Tool) {
			Tool->DestroyTool(this);
		}
	}
	Tools.Reset();
}

void UConstructionSystemComponent::_CreateTool(EConstructionSystemToolType ToolType, TSubclassOf<UConstructionSystemTool> InToolClass)
{
	UConstructionSystemTool* Tool = NewObject<UConstructionSystemTool>(this, InToolClass);
	if (Tool) {
		Tool->InitializeTool(this);
		UConstructionSystemTool*& ToolRef = Tools.FindOrAdd(ToolType);
		ToolRef = Tool;
	}
}

APlayerController* UConstructionSystemComponent::GetPlayerController()
{
	return Cast<APlayerController>(GetOwner());
}

APawn* UConstructionSystemComponent::GetControlledPawn()
{
	APlayerController* PC = GetPlayerController();
	return PC ? PC->GetPawn() : nullptr;
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

	UConstructionSystemTool* ActiveTool = GetActiveTool();
	if (ActiveTool) {
		ActiveTool->Update(this);
	}
}

void UConstructionSystemComponent::BindInput(UInputComponent* InputComponent)
{
	if (InputComponent) {
		InputComponent->BindAction("CSModeToggle", IE_Pressed, this, &UConstructionSystemComponent::ToggleConstructionSystem);
		InputComponent->BindAction("CSToggleBuildUI", IE_Pressed, this, &UConstructionSystemComponent::ToggleBuildUI);
		InputComponent->BindAction<FSetToolDelegate>("CSModeToolBuild", IE_Pressed, this, &UConstructionSystemComponent::EnableConstructionSystem, EConstructionSystemToolType::BuildTool);
		InputComponent->BindAction<FSetToolDelegate>("CSModeToolRemove", IE_Pressed, this, &UConstructionSystemComponent::EnableConstructionSystem, EConstructionSystemToolType::RemoveTool);

		// Bind the tool inputs
		for (auto& Entry : Tools) {
			UConstructionSystemTool* Tool = Entry.Value;
			Tool->BindInput(InputComponent);
		}
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
			PlayerController->SetInputMode(FInputModeGameAndUI());
			PlayerController->bShowMouseCursor = true;

			// Disable the pawn's input
			APawn* ControlledPawn = PlayerController->GetPawn();
			if (ControlledPawn) {
				ControlledPawn->DisableInput(PlayerController);
			}
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
			
			// Enable the pawn's input
			APawn* ControlledPawn = PlayerController->GetPawn();
			if (ControlledPawn) {
				ControlledPawn->EnableInput(PlayerController);
			}

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
		EnableConstructionSystem(ActiveToolType);
	}
}
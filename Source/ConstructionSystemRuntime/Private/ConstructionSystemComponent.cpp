//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "ConstructionSystemComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/Pawn.h"
#include "Engine/World.h"
#include "ConstructionSystemTool.h"
#include "ConstructionSystemBuildTool.h"


UConstructionSystemComponent::UConstructionSystemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
}

void UConstructionSystemComponent::TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bConstructionSystemEnabled) {
		HandleUpdate();
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

	BindInput();
}

void UConstructionSystemComponent::EnableConstructionSystem()
{
	TransitionCameraTo(ConstructionCameraActor, ConstructionCameraTransitionTime, ConstructionCameraTransitionExp);

	if (!ActiveTool) {
		ActiveTool = NewObject<UConstructionSystemBuildTool>(this, "ActiveTool");
		ActiveTool->InitializeTool(this);
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

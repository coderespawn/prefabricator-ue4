//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystem/Tools/ConstructionSystemTool.h"

#include "Components/InputComponent.h"
#include "ConstructionSystemRemoveTool.generated.h"

class UConstructionSystemCursor;
class APrefabActor;

UCLASS(BlueprintType)
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemRemoveTool : public UConstructionSystemTool {
	GENERATED_BODY()
public:
	//~ Begin UConstructionSystemTool Interface
	virtual void InitializeTool(UConstructionSystemComponent* ConstructionComponent) override;
	virtual void DestroyTool(UConstructionSystemComponent* ConstructionComponent) override;
	virtual void OnToolEnable(UConstructionSystemComponent* ConstructionComponent) override;
	virtual void OnToolDisable(UConstructionSystemComponent* ConstructionComponent) override;
	virtual void Update(UConstructionSystemComponent* ConstructionComponent) override;
	//~ End UConstructionSystemTool Interface

protected:
	virtual void RegisterInputCallbacks(UInputComponent* InputComponent) override;
	virtual void UnregisterInputCallbacks(UInputComponent* InputComponent) override;

	UFUNCTION()
	void HandleInput_RemoveAtCursor();

	void RemoveAtCursor();

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ConstructionSystem")
	float TraceDistance = 4000.0f;

private:
	UPROPERTY(Transient)
	TWeakObjectPtr<APrefabActor> FocusedActor;

	ECollisionChannel PrefabSnapChannel;
	bool bCursorFoundHit = false;

private:
	struct FCSRemoveToolInputBindings {
		FInputActionBinding RemoveAtCursor;
	};
	FCSRemoveToolInputBindings InputBindings;

};


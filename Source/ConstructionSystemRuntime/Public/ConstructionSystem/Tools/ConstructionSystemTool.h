//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystemTool.generated.h"

class UInputComponent;
class UConstructionSystemComponent;

UCLASS(BlueprintType)
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemTool : public UObject {
	GENERATED_BODY()
public:
	virtual void InitializeTool(UConstructionSystemComponent* ConstructionComponent);
	virtual void DestroyTool(UConstructionSystemComponent* ConstructionComponent);
	virtual void OnToolEnable(UConstructionSystemComponent* ConstructionComponent);
	virtual void OnToolDisable(UConstructionSystemComponent* ConstructionComponent);
	virtual void Update(UConstructionSystemComponent* ConstructionComponent) {}

	//~ Begin UObject Interface
	virtual bool IsSupportedForNetworking() const override { return true; }
	//~ End UObject Interface

	bool IsToolEnabled() const { return bToolEnabled; }

protected:
	virtual void RegisterInputCallbacks(UInputComponent* InputComponent) {}
	virtual void UnregisterInputCallbacks(UInputComponent* InputComponent) {}

protected:
	UPROPERTY(Transient)
	bool bToolEnabled = false;
};

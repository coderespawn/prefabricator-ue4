//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystemTool.generated.h"

class UInputComponent;
class APawn;

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemTool : public UObject {
	GENERATED_BODY()
public:
	virtual void InitializeTool(APawn* Owner);
	virtual void DestroyTool(APawn* Owner);
	
	virtual void OnToolEnable() {}
	virtual void OnToolDisable() {}

protected:
	virtual void RegisterInputCallbacks(UInputComponent* InputComponent) {}
	virtual void UnregisterInputCallbacks(UInputComponent* InputComponent) {}
};

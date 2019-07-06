//$ Copyright 2015-19, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "ConstructionSystemTool.h"
#include "ConstructionSystemBuildTool.generated.h"

UCLASS()
class CONSTRUCTIONSYSTEMRUNTIME_API UConstructionSystemBuildTool : public UConstructionSystemTool {
	GENERATED_BODY()
public:
	virtual void InitializeTool(APawn* Owner) override;
	virtual void DestroyTool(APawn* Owner) override;

	virtual void OnToolEnable() override;
	virtual void OnToolDisable() override;

protected:
	virtual void RegisterInputCallbacks(UInputComponent* InputComponent) override;
	virtual void UnregisterInputCallbacks(UInputComponent* InputComponent) override;
};

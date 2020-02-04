//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Templates/SubclassOf.h"
#include "PrefabricatorSettings.generated.h"

UENUM(BlueprintType)
enum class EPrefabricatorPivotPosition : uint8
{
	ExtremeLeft,
	ExtremeRight,
	Center
};

/**
 * 
 */
UCLASS(config = Prefabricator, defaultconfig)
class PREFABRICATORRUNTIME_API UPrefabricatorSettings : public UDeveloperSettings
{
	GENERATED_BODY()
public:

	/* Position pivot on extreme corner (bottom, left up), instead of center. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Settings")
	EPrefabricatorPivotPosition PivotPosition;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Settings")
	bool bAllowDynamicUpdate = true;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, config, Category = "Settings", meta = (ConfigRestartRequired = true))
	bool bShowAssetThumbnails = true;

public:
	UPrefabricatorSettings();

	/** Gets the settings container name for the settings, either Project or Editor */
	virtual FName GetContainerName() const;
	/** Gets the category for the settings, some high level grouping like, Editor, Engine, Game...etc. */
	virtual FName GetCategoryName() const;
	/** The unique name for your section of settings, uses the class's FName. */
	virtual FName GetSectionName() const;

#if WITH_EDITOR
	/** Gets the section text, uses the classes DisplayName by default. */
	virtual FText GetSectionText() const;
	/** Gets the description for the section, uses the classes ToolTip by default. */
	virtual FText GetSectionDescription() const;
#endif
};


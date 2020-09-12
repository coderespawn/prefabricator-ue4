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
	UPROPERTY(config, EditAnywhere, Category = "General Settings")
	EPrefabricatorPivotPosition PivotPosition;

	/** Whenever a prefab is saved, update all the other similar prefabs in the scene to reflect this new change */
	UPROPERTY(config, EditAnywhere, Category = "General Settings")
	bool bAllowDynamicUpdate = true;

	/** Whenever a prefab is saved, update all the other similar prefabs in the scene to reflect this new change */
	UPROPERTY(config, EditAnywhere, Category = "General Settings", Meta=(ConfigRestartRequired=true))
	TSet<UClass*> IgnoreBoundingBoxForObjects;
	
	/** Use this angle while saving the prefab asset */
	UPROPERTY(config, EditAnywhere, Category = "Thumbnail")
	float DefaultThumbnailPitch = -11.25;

	/** Use this angle while saving the prefab asset */
	UPROPERTY(config, EditAnywhere, Category = "Thumbnail")
	float DefaultThumbnailYaw = -157.5;

	/** Use this zoom value while saving the prefab asset */
	UPROPERTY(config, EditAnywhere, Category = "Thumbnail")
	float DefaultThumbnailZoom = 0;

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


//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Factories/Factory.h"
#include "PrefabricatorAssetFactory.generated.h"

UCLASS()
class PREFABRICATOREDITOR_API UPrefabricatorAssetFactory : public UFactory {
	GENERATED_UCLASS_BODY()

	// UFactory interface
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};


UCLASS()
class PREFABRICATOREDITOR_API UPrefabricatorAssetCollectionFactory : public UFactory {
	GENERATED_UCLASS_BODY()

		// UFactory interface
		virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
	virtual bool CanCreateNew() const override;
	// End of UFactory interface
};


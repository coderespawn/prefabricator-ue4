//$ Copyright 2015-20, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#pragma once
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Input/Reply.h"

class PREFABRICATOREDITOR_API FPrefabActorCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply HandleSaveToAsset(IDetailLayoutBuilder* DetailBuilder);
	static FReply HandleSaveToNewAsset(IDetailLayoutBuilder* DetailBuilder);
	static FReply HandleLoadFromAsset(IDetailLayoutBuilder* DetailBuilder);
	static FReply RandomizePrefabCollection(IDetailLayoutBuilder* DetailBuilder);
	static FReply UnlinkPrefab(IDetailLayoutBuilder* DetailBuilder);
	static FReply UpdateThumbFromViewport(IDetailLayoutBuilder* DetailBuilder);
};

class PREFABRICATOREDITOR_API FPrefabRandomizerCustomization : public IDetailCustomization {
public:
	// IDetailCustomization interface
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
	// End of IDetailCustomization interface

	static TSharedRef<IDetailCustomization> MakeInstance();

	static FReply HandleRandomize(IDetailLayoutBuilder* DetailBuilder);
};


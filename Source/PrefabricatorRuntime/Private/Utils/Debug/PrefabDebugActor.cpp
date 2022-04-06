//$ Copyright 2015-22, Code Respawn Technologies Pvt Ltd - All Rights Reserved $//

#include "Utils/Debug/PrefabDebugActor.h"

#include "Serialization/ObjectReader.h"
#include "Serialization/ObjectWriter.h"

void APrefabDebugActor::SaveActorData()
{
	if (Actor) {
		UObject* Obj = Actor->GetComponents().Array()[0];
		FObjectWriter ObectWriter(Obj, ActorData);
	}
}

void APrefabDebugActor::LoadActorData()
{
	if (Actor) {
		UObject* Obj = Actor->GetComponents().Array()[0];
		FObjectReader ObectReader(Obj, ActorData);

		Obj->PostLoad();
		Actor->PostLoad();
		Actor->ReregisterAllComponents();
		Actor->PostActorCreated();

#if WITH_EDITOR
		FCoreUObjectDelegates::OnObjectModified.Broadcast(Obj);
		//Obj->PostEditChange();
#endif

		Actor->Modify(true);
	}
}


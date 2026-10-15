// Copyright Torben Borghoff, Keygen-Games

#include "Model/ModelContainer.h"

#include "GameFramework/Actor.h"
#include "KeyCore.h"
#include "Model/KeyCoreModelComponent.h"

UKeyCoreModelComponent* UModelContainer::GetComponentByClass(const TSubclassOf<UKeyCoreModelComponent> ComponentClass)
{
	if (!ensureMsgf(ComponentClass, TEXT("UModelContainer::Get was called without a Sub-Model class.")))
	{
		return nullptr;
	}

	const UClass* ComponentKey = ComponentClass.Get();
	if (const TWeakObjectPtr<UKeyCoreModelComponent>* CachedComponent = ComponentCache.Find(ComponentKey); CachedComponent && CachedComponent->IsValid())
	{
		return CachedComponent->Get();
	}

	const AActor* Owner = Cast<AActor>(GetOuter());
	if (!ensureMsgf(Owner, TEXT("UModelContainer must be a default subobject of the GameState, but its outer is '%s'."), *GetNameSafe(GetOuter())))
	{
		return nullptr;
	}

	UKeyCoreModelComponent* Component = Cast<UKeyCoreModelComponent>(Owner->GetComponentByClass(ComponentClass));
	if (!ensureMsgf(Component, TEXT("Sub-Model '%s' is missing on '%s'. Add it in the GameState constructor."), *ComponentKey->GetName(), *Owner->GetName()))
	{
		return nullptr;
	}

	UE_LOG(LogKeyCoreModel, Verbose, TEXT("Resolved Sub-Model '%s' on '%s'."), *ComponentKey->GetName(), *Owner->GetName());
	ComponentCache.Add(ComponentKey, Component);
	return Component;
}

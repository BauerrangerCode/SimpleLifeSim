// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "UObject/Object.h"

#include "ModelContainer.generated.h"

class UKeyCoreModelComponent;

/**
 * Entry point to the Sub-Models of one GameState, used as GetModel()->Get<UTimeModelComponent>().
 * It holds no game data. The Sub-Models are components of the owning GameState; the container only resolves and caches them,
 * so there is no second source of truth.
 */
UCLASS()
class KEYCORE_API UModelContainer : public UObject
{
	GENERATED_BODY()

public:
	/** Returns the Sub-Model of type T on the owning GameState. Missing Sub-Models are reported with ensure and return nullptr. */
	template <typename T>
	T* Get()
	{
		static_assert(TIsDerivedFrom<T, UKeyCoreModelComponent>::IsDerived, "'T' in UModelContainer::Get must be derived from UKeyCoreModelComponent");
		return Cast<T>(GetComponentByClass(T::StaticClass()));
	}

private:
	/** Finds the Sub-Model by class on the owner and caches it. The cache is valid because default subobjects never change. */
	UKeyCoreModelComponent* GetComponentByClass(const TSubclassOf<UKeyCoreModelComponent> ComponentClass);

	/** Resolved Sub-Models by class. Weak, because the components are owned by the GameState. */
	TMap<const UClass*, TWeakObjectPtr<UKeyCoreModelComponent>> ComponentCache;
};

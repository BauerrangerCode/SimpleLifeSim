// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "UObject/Interface.h"

#include "KeyCoreModelProvider.generated.h"

class UModelContainer;

/** Reflection stub of IKeyCoreModelProvider. NotBlueprintable, because the Model is set up in C++ on the GameState. */
UINTERFACE(MinimalAPI, NotBlueprintable)
class UKeyCoreModelProvider : public UInterface
{
	GENERATED_BODY()
};

/**
 * Implemented by the game's GameState to hand its Model to the core framework. KeyCore never names the game's GameState class,
 * so the dependency direction stays game module -> plugin.
 */
class KEYCORE_API IKeyCoreModelProvider
{
	GENERATED_BODY()

public:
	/** Returns the Model of this GameState. Null means the GameState is not set up yet and Controllers must not start. */
	virtual UModelContainer* GetModel() const = 0;
};

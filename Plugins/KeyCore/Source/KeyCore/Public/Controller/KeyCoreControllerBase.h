// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "UObject/Object.h"

#include "KeyCoreControllerBase.generated.h"

class UModelContainer;

/** Input that the ControllerOrchestrator passes to every Controller on initialization. */
struct FControllerContext
{
	/** Model of the GameState. Controllers write to it and never keep game state of their own. */
	UModelContainer* Model = nullptr;
};

/**
 * Base of all server-side Controllers. A Controller applies game rules and is the only layer that writes to the Model.
 * The ControllerOrchestrator owns, orders, initializes, ticks and deinitializes Controllers.
 */
UCLASS(Abstract)
class KEYCORE_API UControllerBase : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * Called once on the authority. The dependencies from GetDependencies() were delivered through ReceiveDependency before this call.
	 * Overrides must call Super first.
	 */
	virtual void Initialize(const FControllerContext& Context);

	/** Called once when the Orchestrator stops, in reverse initialization order. Overrides must call Super last. */
	virtual void Deinitialize();

	/** Called every frame, only if the Controller opted in with bCanTick. Most Controllers stay event-driven (CLAUDE.md 7.5). */
	virtual void Tick(float DeltaSeconds);

	/** Controllers this one needs. The Orchestrator checks that each of them is initialized before this Controller. */
	virtual TArray<TSubclassOf<UControllerBase>> GetDependencies() const;

	/** Receives an initialized dependency. Typed access goes through an interface the dependency implements, never a hard cast. */
	virtual void ReceiveDependency(UControllerBase* Dependency);

	/** Model read access for the View layer and tests. Writes still happen only inside Controllers. */
	UModelContainer* GetModel() const;

	bool CanTick() const;

protected:
	/** Opt-in to Tick. Off by default, so a Controller without per-frame work costs nothing. */
	bool bCanTick = false;

private:
	/** Model of the GameState. Kept as UPROPERTY so the garbage collector tracks it. */
	UPROPERTY()
	TObjectPtr<UModelContainer> Model;
};

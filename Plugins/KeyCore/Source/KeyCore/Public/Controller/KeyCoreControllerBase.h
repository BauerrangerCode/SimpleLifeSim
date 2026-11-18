// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "UObject/Object.h"

#include "KeyCoreControllerBase.generated.h"

class UModelContainer;

/** Input that the ControllerOrchestrator passes to every Controller on initialization. */
USTRUCT()
struct FControllerContext
{
	GENERATED_BODY()

	/** Model of the GameState. Controllers write to it and never keep game state of their own. */
	UPROPERTY()
	TObjectPtr<UModelContainer> Model;
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

	/** Model this Controller writes to. Public so that automation specs can check the injected Model; Views never hold Controllers. */
	UModelContainer* GetModel() const;

	/** Asked by the Orchestrator every frame, so only Controllers that opted in with bCanTick are ticked. */
	bool CanTick() const;

protected:
	/** Opt-in to Tick. Off by default, so a Controller without per-frame work costs nothing. */
	bool bCanTick = false;

private:
	/** Model of the GameState. Kept as UPROPERTY so the garbage collector tracks it. */
	UPROPERTY()
	TObjectPtr<UModelContainer> Model;
};

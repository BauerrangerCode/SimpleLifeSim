// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Subsystems/WorldSubsystem.h"

#include "KeyCoreControllerOrchestrator.generated.h"

class UControllerBase;
class UModelContainer;

/**
 * Owns and runs the Controllers on the authority. It is a world subsystem, so clients have it too, but on clients it stays
 * inactive: it creates no Controllers and never ticks. The central tick replaces one tick per Controller (MVVM plan, D2).
 */
UCLASS()
class KEYCORE_API UControllerOrchestrator : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	//~ Begin UWorldSubsystem Interface
	virtual void Deinitialize() override;
	//~ End UWorldSubsystem Interface

	/**
	 * Starts the Controllers on the authority. The GameState calls it once its Model is ready (MVVM plan, D23), so every Controller
	 * finds the Model in place. Does nothing on clients and in worlds without a GameMode.
	 */
	void StartControllers();

	//~ Begin FTickableGameObject Interface
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface

	/** Initialized Controllers in initialization order. Empty on clients and before the world begins play. */
	const TArray<TObjectPtr<UControllerBase>>& GetControllers() const;

	/**
	 * Checks that every entry is a concrete Controller class listed once, and that every dependency is listed before its dependent. Reads only the class defaults,
	 * so nothing is spawned. Returns an empty string if the order is valid, otherwise the first problem found.
	 */
	static FString ValidateDependencyOrder(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses);

	/**
	 * Creates and initializes the given Controller classes in list order and delivers their dependencies.
	 * Returns false after logging and ensuring the cause if the order is invalid or Controllers already run. Nothing is initialized in that case.
	 */
	bool InitializeControllers(UModelContainer* Model, const TArray<TSubclassOf<UControllerBase>>& ControllerClasses);

private:
	/** Checks one settings entry on its own: it must be set, concrete, and listed only once. Returns the problem or an empty string. */
	static FString ValidateControllerEntry(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses, int32 Index);

	/** Checks that every dependency of one entry is set, listed, not the entry itself, and listed before it. Returns the problem or an empty string. */
	static FString ValidateDependenciesOfEntry(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses, int32 Index);

	/** Delivers every dependency of Controller from the already initialized Controllers. Validation guarantees they exist. */
	void DeliverDependencies(UControllerBase& Controller);

	/** Deinitializes in reverse initialization order, so no Controller outlives a dependency it still uses. */
	void DeinitializeControllers();

	/** Logs and ensures a start-up error. Logged too, because an ensure fires only once per call site and session. */
	static void ReportInitializationError(const FString& Message);

	/** Owned Controllers in initialization order. UPROPERTY, so the garbage collector keeps them alive. */
	UPROPERTY()
	TArray<TObjectPtr<UControllerBase>> Controllers;

	/** True only after the Controllers were initialized successfully on the authority. Gates the tick. */
	bool bIsActive = false;
};

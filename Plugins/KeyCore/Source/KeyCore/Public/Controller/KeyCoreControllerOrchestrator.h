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
	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;
	//~ End UWorldSubsystem Interface

	//~ Begin FTickableGameObject Interface
	virtual bool IsTickable() const override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	//~ End FTickableGameObject Interface

	/** Initialized Controllers in initialization order. Empty on clients and before the world begins play. */
	const TArray<TObjectPtr<UControllerBase>>& GetControllers() const;

	/**
	 * Checks that every dependency of the listed Controller classes is listed before its dependent. Reads only the class defaults,
	 * so nothing is spawned. Returns an empty string if the order is valid, otherwise the first problem found.
	 */
	static FString ValidateDependencyOrder(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses);

	/**
	 * Creates and initializes the given Controller classes in list order and delivers their dependencies.
	 * Returns false after logging and ensuring the cause if the order is invalid. Nothing is initialized in that case.
	 */
	bool InitializeControllers(UModelContainer* Model, const TArray<TSubclassOf<UControllerBase>>& ControllerClasses);

private:
	/** Delivers every dependency of Controller from the already initialized Controllers. Validation guarantees they exist. */
	void DeliverDependencies(UControllerBase& Controller);

	void DeinitializeControllers();

	UPROPERTY()
	TArray<TObjectPtr<UControllerBase>> Controllers;

	/** True only after the Controllers were initialized successfully on the authority. Gates the tick. */
	bool bIsActive = false;
};

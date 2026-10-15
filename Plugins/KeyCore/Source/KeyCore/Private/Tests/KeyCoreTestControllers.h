// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Controller/KeyCoreControllerBase.h"

#include "KeyCoreTestControllers.generated.h"

/*
 * Test-only Controllers. They are compiled in every configuration, because UHT does not allow UCLASS inside preprocessor blocks.
 * HideDropdown keeps them out of the Controller picker in Project Settings.
 */

/** Test-only Controller without dependencies. */
UCLASS(HideDropdown)
class UKeyCoreTestControllerA : public UControllerBase
{
	GENERATED_BODY()
};

/** Test-only Controller that depends on UKeyCoreTestControllerA and records the delivered dependency. */
UCLASS(HideDropdown)
class UKeyCoreTestControllerB : public UControllerBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UControllerBase>> GetDependencies() const override
	{
		return { UKeyCoreTestControllerA::StaticClass() };
	}

	virtual void ReceiveDependency(UControllerBase* Dependency) override
	{
		ReceivedDependency = Dependency;
	}

	/** Dependency delivered by the Orchestrator. Must be the initialized UKeyCoreTestControllerA. */
	UPROPERTY()
	TObjectPtr<UControllerBase> ReceivedDependency;
};

/** Test-only Controller that lists itself as dependency, which validation must reject. */
UCLASS(HideDropdown)
class UKeyCoreTestControllerSelfDependent : public UControllerBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UControllerBase>> GetDependencies() const override
	{
		return { UKeyCoreTestControllerSelfDependent::StaticClass() };
	}
};

/** Test-only Controller that returns an empty dependency class, which validation must reject without crashing. */
UCLASS(HideDropdown)
class UKeyCoreTestControllerEmptyDependency : public UControllerBase
{
	GENERATED_BODY()

public:
	virtual TArray<TSubclassOf<UControllerBase>> GetDependencies() const override
	{
		return { TSubclassOf<UControllerBase>() };
	}
};

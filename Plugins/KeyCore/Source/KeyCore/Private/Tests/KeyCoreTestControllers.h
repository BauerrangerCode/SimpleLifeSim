// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Controller/KeyCoreControllerBase.h"

#include "KeyCoreTestControllers.generated.h"

/*
 * Test-only Controllers. They exist only where editor-only data is compiled (WITH_EDITORONLY_DATA), so they are absent
 * from dedicated-server and shipping builds. UHT accepts UCLASS inside this one guard, but not inside other guards.
 * HideDropdown keeps them out of the Controller picker in Project Settings.
 */
#if WITH_EDITORONLY_DATA

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
	KEYCORE_CONTROLLER_ORDER(TDependsOn<UKeyCoreTestControllerA>)

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

#endif // WITH_EDITORONLY_DATA

// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Controller/KeyCoreControllerBase.h"

#include "KeyCoreTestControllers.generated.h"

/** Test-only Controller without dependencies. Compiled in every configuration, because UHT does not allow UCLASS inside preprocessor blocks. */
UCLASS()
class UKeyCoreTestControllerA : public UControllerBase
{
	GENERATED_BODY()
};

/** Test-only Controller that depends on UKeyCoreTestControllerA and records the delivered dependency. */
UCLASS()
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

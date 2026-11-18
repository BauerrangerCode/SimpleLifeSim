// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "GameFramework/GameStateBase.h"
#include "Model/KeyCoreModelProvider.h"

#include "SimpleLifeSimGameState.generated.h"

class UModelContainer;

/** Log category of the GameState, which owns the Model registration of the game. */
DECLARE_LOG_CATEGORY_EXTERN(LogSimpleLifeSimGameState, Log, All);

/**
 * GameState of the game. It owns the Model: the replicated game data that both co-op players read.
 * Sub-Models are added as default subobjects in the constructor, so they exist on clients before the first replication.
 */
UCLASS()
class ASimpleLifeSimGameState : public AGameStateBase, public IKeyCoreModelProvider
{
	GENERATED_BODY()

public:
	ASimpleLifeSimGameState();

	//~ Begin IKeyCoreModelProvider Interface
	virtual UModelContainer* GetModel() const override;
	//~ End IKeyCoreModelProvider Interface

protected:
	//~ Begin AGameStateBase Interface
	virtual void HandleBeginPlay() override;
	virtual void OnRep_ReplicatedHasBegunPlay() override;
	//~ End AGameStateBase Interface

private:
	/**
	 * Single registration point for the model-ready state. The engine calls it on the server through HandleBeginPlay
	 * and on clients through OnRep_ReplicatedHasBegunPlay, so no separate flag is needed.
	 */
	void RegisterModelReady();

	/** Access point for Sub-Models. Created with the GameState, so it exists before any replication arrives. */
	UPROPERTY()
	TObjectPtr<UModelContainer> Model;
};

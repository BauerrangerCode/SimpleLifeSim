// Copyright Torben Borghoff, Keygen-Games

#include "GameState/SimpleLifeSimGameState.h"

#include "Model/ModelContainer.h"
#include "Time/TimeModelComponent.h"

DEFINE_LOG_CATEGORY(LogSimpleLifeSimGameState);

ASimpleLifeSimGameState::ASimpleLifeSimGameState()
{
	Model = CreateDefaultSubobject<UModelContainer>(TEXT("Model"));

	// Sub-Models are listed here, one line each (plan D4). The container finds them by class.
	CreateDefaultSubobject<UTimeModelComponent>(TEXT("TimeModel"));
}

UModelContainer* ASimpleLifeSimGameState::GetModel() const
{
	return Model;
}

void ASimpleLifeSimGameState::HandleBeginPlay()
{
	Super::HandleBeginPlay();
	RegisterModelReady();
}

void ASimpleLifeSimGameState::OnRep_ReplicatedHasBegunPlay()
{
	Super::OnRep_ReplicatedHasBegunPlay();
	RegisterModelReady();
}

void ASimpleLifeSimGameState::RegisterModelReady()
{
	UE_LOG(LogSimpleLifeSimGameState, Log, TEXT("Model of '%s' is ready on %s."), *GetName(), HasAuthority() ? TEXT("the server") : TEXT("a client"));
}

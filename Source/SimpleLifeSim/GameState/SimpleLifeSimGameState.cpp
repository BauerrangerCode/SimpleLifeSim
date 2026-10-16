// Copyright Torben Borghoff, Keygen-Games

#include "GameState/SimpleLifeSimGameState.h"

#include "Model/ModelContainer.h"

DEFINE_LOG_CATEGORY(LogSimpleLifeSimGameState);

ASimpleLifeSimGameState::ASimpleLifeSimGameState()
{
	Model = CreateDefaultSubobject<UModelContainer>(TEXT("Model"));
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

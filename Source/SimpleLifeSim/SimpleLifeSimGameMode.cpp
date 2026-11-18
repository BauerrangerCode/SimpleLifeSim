// Copyright Epic Games, Inc. All Rights Reserved.

#include "SimpleLifeSimGameMode.h"

#include "GameState/SimpleLifeSimGameState.h"

ASimpleLifeSimGameMode::ASimpleLifeSimGameMode()
{
	GameStateClass = ASimpleLifeSimGameState::StaticClass();
}

// Copyright Torben Borghoff, Keygen-Games

#include "Time/TimeController.h"

#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Time/TimeModelComponent.h"
#include "Time/TimeSettings.h"
#include "Model/ModelContainer.h"

UTimeController::UTimeController()
{
	// The clock must advance every frame, so this Controller opts in to the central tick.
	bCanTick = true;
}

void UTimeController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UTimeModelComponent* TimeModel = GetModel()->Get<UTimeModelComponent>();
	AGameStateBase* GameState = GetWorld()->GetGameState();
	if (!ensureMsgf(TimeModel && GameState, TEXT("TimeController needs the time Model and a GameState.")))
	{
		return;
	}

	const UTimeSettings* Settings = GetDefault<UTimeSettings>();
	const int32 CurrentMinuteOfDay = CalculateMinuteOfDay(GameState->GetServerWorldTimeSeconds(), Settings->StartMinuteOfDay, Settings->InGameMinutesPerRealSecond);
	if (CurrentMinuteOfDay != TimeModel->GetMinuteOfDay())
	{
		UE_LOG(LogSimpleLifeSimTime, Verbose, TEXT("Minute of day written on the server: %d."), CurrentMinuteOfDay);
		TimeModel->SetMinuteOfDay(CurrentMinuteOfDay);
	}
}

int32 UTimeController::CalculateMinuteOfDay(double ServerSeconds, int32 StartMinuteOfDay, float InGameMinutesPerRealSecond)
{
	const int64 ElapsedInGameMinutes = FMath::FloorToInt64(StartMinuteOfDay + ServerSeconds * InGameMinutesPerRealSecond);
	return static_cast<int32>(ElapsedInGameMinutes % UTimeModelComponent::MinutesPerDay);
}

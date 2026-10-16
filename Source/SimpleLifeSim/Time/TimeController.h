// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Controller/KeyCoreControllerBase.h"

#include "TimeController.generated.h"

/**
 * Advances the in-game clock on the server. It derives the minute of the day from the server world time, so no state is kept
 * in the Controller, and it writes the Model only when the minute changes (about once per in-game minute).
 */
UCLASS()
class UTimeController : public UControllerBase
{
	GENERATED_BODY()

public:
	UTimeController();

	virtual void Tick(float DeltaSeconds) override;

	/** Pure calculation of the minute of the day, kept free of world access so that it can be tested directly. */
	static int32 CalculateMinuteOfDay(double ServerSeconds, int32 StartMinuteOfDay, float InGameMinutesPerRealSecond);
};

// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Engine/DeveloperSettings.h"

#include "TimeSettings.generated.h"

/**
 * Project-wide tuning of the in-game clock. Designers change the start time and the speed here, so the speed is never
 * hard-coded in the TimeController.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Time"))
class UTimeSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/** Minute of the day at which the game clock starts, from 0 to 1439. The C++ default is only a starting point, the authoritative value is in the editor. */
	UPROPERTY(Config, EditAnywhere, Category = "Time", meta = (ClampMin = "0", ClampMax = "1439", Units = "min"))
	int32 StartMinuteOfDay = 480;

	/** In-game minutes that pass per real second on the server. A value of 1.0 means one in-game minute per real second. */
	UPROPERTY(Config, EditAnywhere, Category = "Time", meta = (ClampMin = "0.0"))
	float InGameMinutesPerRealSecond = 1.0f;
};

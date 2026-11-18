// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "FieldNotificationDeclaration.h"
#include "Model/KeyCoreModelComponent.h"

#include "TimeModelComponent.generated.h"

/** Log category of the in-game clock: Model, Controller and ViewModel of the time slice. */
DECLARE_LOG_CATEGORY_EXTERN(LogSimpleLifeSimTime, Log, All);

/**
 * Model of the in-game clock. It stores only the minute of the day, because the clock is shared by all players and
 * one minute of game time is the finest step a player can see. The value therefore replicates about once per in-game minute.
 * Only the TimeController writes it, on the server.
 */
UCLASS()
class UTimeModelComponent : public UKeyCoreModelComponent
{
	GENERATED_BODY()

public:
	/** Minutes in one in-game hour. Fixed by the calendar, so it is a constant and not an editor setting. */
	static constexpr int32 MinutesPerHour = 60;

	/** Minutes in one in-game day. The clock wraps around at this value. */
	static constexpr int32 MinutesPerDay = 24 * MinutesPerHour;

	UTimeModelComponent(const FObjectInitializer& ObjectInitializer);

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** Minute of the day from 0 to MinutesPerDay - 1. Read by ViewModels and by the TimeController to detect a change. */
	int32 GetMinuteOfDay() const;

	/** Controller-only write (convention, see KeyCoreModelComponent). Notifies FieldNotify subscribers only when the minute changes. */
	void SetMinuteOfDay(int32 NewMinuteOfDay);

private:
	/** Replication callback. Clients broadcast FieldNotify here, because the value changed without a local setter call. */
	UFUNCTION()
	void OnRep_MinuteOfDay();

	/** Minute of the day. FieldNotify, because the time widget shows it; replicated, because every client shows the same clock. */
	UPROPERTY(ReplicatedUsing = OnRep_MinuteOfDay, FieldNotify)
	int32 MinuteOfDay = 0;
};

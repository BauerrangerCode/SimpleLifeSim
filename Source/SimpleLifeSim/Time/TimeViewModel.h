// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "MVVMViewModelBase.h"

#include "TimeViewModel.generated.h"

class UTimeModelComponent;

/**
 * Presents the in-game clock to the UI as localized text. It reads the Model and never writes it, and it shows a placeholder
 * while no Model is bound, which is the normal state before replication has arrived.
 */
UCLASS(BlueprintType)
class UTimeViewModel : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	/** Connects the ViewModel to the Model. Called once by the resolver. A null Model keeps the placeholder text. */
	void BindModel(UTimeModelComponent* InTimeModel);

	/** Day time as text, such as "08:00", or a placeholder while the Model is not available. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "Time")
	FText DayTimeText;

private:
	/** Model change callback. The Model notifies this ViewModel, so the View never polls. */
	void HandleMinuteOfDayChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId);

	/** Formats the current minute into DayTimeText. */
	void UpdateDayTimeText();

	/** Weak, because the GameState owns the Model. The ViewModel must not keep it alive. */
	TWeakObjectPtr<UTimeModelComponent> TimeModel;
};

// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Blueprint/UserWidget.h"

#include "TimeWidget.generated.h"

class UTextBlock;

/**
 * Base class of the day-time widget. It contains no game logic: the Blueprint child binds DayTimeText of TimeViewModel to the
 * text block through the MVVM binding editor. The base class only declares which text block the Blueprint must provide.
 */
UCLASS(Abstract)
class UTimeWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	/** Shows the day time. The name must match the text block in the Blueprint, because BindWidget links them by name. */
	UPROPERTY(BlueprintReadOnly, meta = (BindWidget))
	TObjectPtr<UTextBlock> DayTimeTextBlock;
};

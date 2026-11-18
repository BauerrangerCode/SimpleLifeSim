// Copyright Torben Borghoff, Keygen-Games

#include "Time/TimeViewModel.h"

#include "Internationalization/Text.h"
#include "Time/TimeModelComponent.h"

#define LOCTEXT_NAMESPACE "TimeViewModel"

namespace
{
	/** Hours and minutes are shown with two digits, so 8 o'clock reads as 08:00. */
	constexpr int32 DigitsPerTimeField = 2;
}

void UTimeViewModel::BindModel(UTimeModelComponent* InTimeModel)
{
	TimeModel = InTimeModel;
	if (InTimeModel)
	{
		InTimeModel->AddFieldValueChangedDelegate(UTimeModelComponent::FFieldNotificationClassDescriptor::MinuteOfDay,
			UTimeModelComponent::FFieldValueChangedDelegate::CreateUObject(this, &UTimeViewModel::HandleMinuteOfDayChanged));
	}
	UpdateDayTimeText();
}

void UTimeViewModel::HandleMinuteOfDayChanged(UObject* Source, UE::FieldNotification::FFieldId FieldId)
{
	UpdateDayTimeText();
}

void UTimeViewModel::UpdateDayTimeText()
{
	const UTimeModelComponent* Model = TimeModel.Get();
	if (!Model)
	{
		UE_MVVM_SET_PROPERTY_VALUE(DayTimeText, LOCTEXT("DayTimePlaceholder", "--:--"));
		return;
	}

	const int32 Hours = Model->GetMinuteOfDay() / UTimeModelComponent::MinutesPerHour;
	const int32 Minutes = Model->GetMinuteOfDay() % UTimeModelComponent::MinutesPerHour;

	FNumberFormattingOptions TwoDigits;
	TwoDigits.MinimumIntegralDigits = DigitsPerTimeField;
	UE_MVVM_SET_PROPERTY_VALUE(DayTimeText, FText::Format(LOCTEXT("DayTime", "{0}:{1}"), FText::AsNumber(Hours, &TwoDigits), FText::AsNumber(Minutes, &TwoDigits)));
}

#undef LOCTEXT_NAMESPACE

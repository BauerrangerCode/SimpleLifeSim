// Copyright Torben Borghoff, Keygen-Games

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Time/TimeController.h"

BEGIN_DEFINE_SPEC(FTimeControllerSpec, "SimpleLifeSim.Time", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FTimeControllerSpec)

void FTimeControllerSpec::Define()
{
	Describe(TEXT("CalculateMinuteOfDay"), [this]()
	{
		It(TEXT("starts at the configured minute"), [this]()
		{
			TestEqual(TEXT("No elapsed time keeps the start minute"), UTimeController::CalculateMinuteOfDay(0.0, 480, 1.0f), 480);
		});

		It(TEXT("advances by the configured speed"), [this]()
		{
			TestEqual(TEXT("60 real seconds at one minute per second add 60 minutes"), UTimeController::CalculateMinuteOfDay(60.0, 480, 1.0f), 540);
			TestEqual(TEXT("30 real seconds at two minutes per second add 60 minutes"), UTimeController::CalculateMinuteOfDay(30.0, 0, 2.0f), 60);
		});

		It(TEXT("wraps around at the end of the day"), [this]()
		{
			TestEqual(TEXT("One minute after 23:59 is 00:00"), UTimeController::CalculateMinuteOfDay(1.0, 1439, 1.0f), 0);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS

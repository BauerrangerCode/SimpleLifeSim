// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "CoreMinimal.h"
#include "FieldNotificationDeclaration.h"
#include "Model/KeyCoreModelComponent.h"

#include "KeyCoreTestModelComponent.generated.h"

/**
 * Test-only Sub-Model with one FieldNotify value, used by the KeyCore automation specs.
 * UHT forbids guarding UCLASS with preprocessor blocks, so this class is compiled in every configuration.
 * HideDropdown keeps it out of component pickers in the editor.
 */
UCLASS(HideDropdown)
class UKeyCoreTestModelComponent : public UKeyCoreModelComponent
{
	GENERATED_BODY()

public:
	/** Writes through the same SetPropertyValue path as production Sub-Models, so the FieldNotify behavior is what is tested. */
	void SetTestValue(int32 NewValue)
	{
		SetPropertyValue(TestValue, NewValue, ThisClass::FFieldNotificationClassDescriptor::TestValue);
	}

	/** Value that must notify FieldNotify subscribers only when it actually changes. */
	UPROPERTY(BlueprintReadOnly, FieldNotify, Category = "KeyCore|Test")
	int32 TestValue = 0;
};

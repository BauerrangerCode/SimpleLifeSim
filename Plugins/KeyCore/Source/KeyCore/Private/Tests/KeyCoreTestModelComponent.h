// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "CoreMinimal.h"
#include "FieldNotificationDeclaration.h"
#include "Model/KeyCoreModelComponent.h"

#include "KeyCoreTestModelComponent.generated.h"

#if WITH_EDITORONLY_DATA

/**
 * Test-only Sub-Model with one FieldNotify value, used by the KeyCore automation specs.
 * It exists only where editor-only data is compiled (WITH_EDITORONLY_DATA), so it is absent from dedicated-server and shipping builds.
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

#endif // WITH_EDITORONLY_DATA

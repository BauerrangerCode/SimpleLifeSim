// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "View/MVVMViewModelContextResolver.h"

#include "TimeViewModelResolver.generated.h"

/**
 * Creates the TimeViewModel for a widget and binds it to the time Model of the GameState. The MVVM plugin calls it in place of
 * a hard-wired reference, so the widget does not search for the GameState itself.
 */
UCLASS()
class UTimeViewModelResolver : public UMVVMViewModelContextResolver
{
	GENERATED_BODY()

public:
	virtual UObject* CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const override;
};

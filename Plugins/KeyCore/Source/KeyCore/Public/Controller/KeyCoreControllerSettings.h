// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Engine/DeveloperSettings.h"

#include "KeyCoreControllerSettings.generated.h"

class UControllerBase;

/**
 * Project-wide list of Controllers. It lives in Project Settings so that designers add or remove Controllers without code changes.
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "KeyCore Controllers"))
class KEYCORE_API UControllerSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	/**
	 * Controllers in initialization order. The tick order is the same list order. The list order is an interim order
	 * until the order-of-operations macro exists (decision D1 in the MVVM plan).
	 */
	UPROPERTY(Config, EditAnywhere, Category = "Controllers")
	TArray<TSoftClassPtr<UControllerBase>> Controllers;
};

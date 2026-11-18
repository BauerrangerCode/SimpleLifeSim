// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Logging/LogMacros.h"
#include "Modules/ModuleManager.h"

/** Log category of the Model layer (Sub-Models, model access). */
DECLARE_LOG_CATEGORY_EXTERN(LogKeyCoreModel, Log, All);

/** Log category of the Controller layer (Orchestrator, dependency order, Controller lifecycle). */
DECLARE_LOG_CATEGORY_EXTERN(LogKeyCoreController, Log, All);

/** Module entry point of the KeyCore plugin. Holds no state; the framework classes are added in later steps. */
class FKeyCoreModule : public IModuleInterface
{
};

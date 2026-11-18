// Copyright Torben Borghoff, Keygen-Games

#include "Controller/KeyCoreControllerBase.h"
#include "Controller/KeyCoreControllerOrchestrator.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "KeyCore.h"
#include "Model/KeyCoreModelComponent.h"
#include "UObject/UnrealType.h"

namespace
{
	/** Lists the Controllers in the order in which the Orchestrator initializes and ticks them. */
	void PrintControllerOrder(const UWorld& World)
	{
		const UControllerOrchestrator* Orchestrator = World.GetSubsystem<UControllerOrchestrator>();
		if (!Orchestrator)
		{
			UE_LOG(LogKeyCoreController, Log, TEXT("No Controller Orchestrator exists in this world."));
			return;
		}

		const TArray<TObjectPtr<UControllerBase>>& Controllers = Orchestrator->GetControllers();
		UE_LOG(LogKeyCoreController, Log, TEXT("Controller order: %d entries, orchestrator active: %s."), Controllers.Num(), Orchestrator->IsTickable() ? TEXT("yes") : TEXT("no"));
		for (int32 Index = 0; Index < Controllers.Num(); ++Index)
		{
			UE_LOG(LogKeyCoreController, Log, TEXT("  %d: %s (ticks: %s)"), Index, *Controllers[Index]->GetClass()->GetName(), Controllers[Index]->CanTick() ? TEXT("yes") : TEXT("no"));
		}
	}

	/** Prints every property of every Sub-Model on the GameState, as this machine currently holds it. */
	void PrintModelState(const UWorld& World)
	{
		const AGameStateBase* GameState = World.GetGameState();
		if (!GameState)
		{
			UE_LOG(LogKeyCoreModel, Log, TEXT("No GameState exists in this world, so there is no Model."));
			return;
		}

		TArray<UKeyCoreModelComponent*> SubModels;
		GameState->GetComponents(SubModels);
		for (UKeyCoreModelComponent* SubModel : SubModels)
		{
			UE_LOG(LogKeyCoreModel, Log, TEXT("Sub-Model %s:"), *SubModel->GetClass()->GetName());
			for (TFieldIterator<FProperty> It(SubModel->GetClass(), EFieldIteratorFlags::ExcludeSuper); It; ++It)
			{
				FString Value;
				It->ExportTextItem_InContainer(Value, SubModel, nullptr, SubModel, PPF_None);
				UE_LOG(LogKeyCoreModel, Log, TEXT("  %s = %s"), *It->GetName(), *Value);
			}
		}
	}

	/** Console entry point. Prints the Model state first, then the Controller order, so both appear in one block of output. */
	void PrintState(const TArray<FString>& Arguments, UWorld* World)
	{
		if (!World)
		{
			UE_LOG(LogKeyCoreModel, Log, TEXT("KeyCore.PrintState needs a world. Run it while a map is loaded."));
			return;
		}

		PrintModelState(*World);
		PrintControllerOrder(*World);
	}

	FAutoConsoleCommandWithWorldAndArgs PrintStateCommand(
		TEXT("KeyCore.PrintState"),
		TEXT("Prints the Sub-Model state and the Controller order of the current world."),
		FConsoleCommandWithWorldAndArgsDelegate::CreateStatic(&PrintState));
}

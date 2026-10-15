// Copyright Torben Borghoff, Keygen-Games

#include "Controller/KeyCoreControllerOrchestrator.h"

#include "Controller/KeyCoreControllerBase.h"
#include "Controller/KeyCoreControllerSettings.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "KeyCore.h"
#include "Model/KeyCoreModelProvider.h"
#include "Model/ModelContainer.h"

void UControllerOrchestrator::OnWorldBeginPlay(UWorld& InWorld)
{
	Super::OnWorldBeginPlay(InWorld);

	// Only the authority has a GameMode. Clients also have this subsystem, which stays inactive for them.
	if (!InWorld.GetAuthGameMode())
	{
		return;
	}

	TArray<TSubclassOf<UControllerBase>> ControllerClasses;
	for (const TSoftClassPtr<UControllerBase>& SoftControllerClass : GetDefault<UControllerSettings>()->Controllers)
	{
		UClass* ControllerClass = SoftControllerClass.LoadSynchronous();
		if (!ensureMsgf(ControllerClass, TEXT("UControllerSettings lists a Controller that cannot be loaded: '%s'."), *SoftControllerClass.ToString()))
		{
			return;
		}
		ControllerClasses.Add(ControllerClass);
	}

	IKeyCoreModelProvider* ModelProvider = Cast<IKeyCoreModelProvider>(InWorld.GetGameState());
	if (!ensureMsgf(ModelProvider, TEXT("The GameState does not implement IKeyCoreModelProvider, so Controllers cannot start.")))
	{
		return;
	}

	bIsActive = InitializeControllers(ModelProvider->GetModel(), ControllerClasses);
}

void UControllerOrchestrator::Deinitialize()
{
	DeinitializeControllers();
	bIsActive = false;
	Super::Deinitialize();
}

bool UControllerOrchestrator::IsTickable() const
{
	return bIsActive;
}

void UControllerOrchestrator::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	for (const TObjectPtr<UControllerBase>& Controller : Controllers)
	{
		if (Controller->CanTick())
		{
			Controller->Tick(DeltaTime);
		}
	}
}

TStatId UControllerOrchestrator::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UControllerOrchestrator, STATGROUP_Tickables);
}

const TArray<TObjectPtr<UControllerBase>>& UControllerOrchestrator::GetControllers() const
{
	return Controllers;
}

FString UControllerOrchestrator::ValidateDependencyOrder(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses)
{
	for (int32 Index = 0; Index < ControllerClasses.Num(); ++Index)
	{
		FString Error = ValidateControllerEntry(ControllerClasses, Index);
		if (Error.IsEmpty())
		{
			Error = ValidateDependenciesOfEntry(ControllerClasses, Index);
		}

		if (!Error.IsEmpty())
		{
			return Error;
		}
	}

	return FString();
}

FString UControllerOrchestrator::ValidateControllerEntry(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses, const int32 Index)
{
	const TSubclassOf<UControllerBase>& ControllerClass = ControllerClasses[Index];
	if (!ControllerClass)
	{
		return FString::Printf(TEXT("UControllerSettings contains an empty Controller entry at index %d."), Index);
	}

	if (ControllerClass->HasAnyClassFlags(CLASS_Abstract))
	{
		return FString::Printf(TEXT("'%s' at index %d is abstract and cannot be created. List a concrete Controller class instead."), *ControllerClass->GetName(), Index);
	}

	const int32 FirstIndex = ControllerClasses.IndexOfByKey(ControllerClass);
	if (FirstIndex != Index)
	{
		return FString::Printf(TEXT("'%s' is listed twice in UControllerSettings.Controllers (index %d and %d). Every Controller exists once."), *ControllerClass->GetName(), FirstIndex, Index);
	}

	return FString();
}

FString UControllerOrchestrator::ValidateDependenciesOfEntry(const TArray<TSubclassOf<UControllerBase>>& ControllerClasses, const int32 Index)
{
	const UClass* ControllerClass = ControllerClasses[Index];
	const UControllerBase* Defaults = ControllerClass->GetDefaultObject<UControllerBase>();
	for (const TSubclassOf<UControllerBase>& DependencyClass : Defaults->GetDependencies())
	{
		if (!DependencyClass)
		{
			return FString::Printf(TEXT("'%s' returns an empty class from GetDependencies()."), *ControllerClass->GetName());
		}

		const int32 ProviderIndex = ControllerClasses.IndexOfByPredicate([&DependencyClass](const TSubclassOf<UControllerBase>& Listed)
		{
			return Listed && Listed->IsChildOf(DependencyClass.Get());
		});

		if (ProviderIndex == INDEX_NONE)
		{
			return FString::Printf(TEXT("'%s' depends on '%s', which is missing from UControllerSettings.Controllers."), *ControllerClass->GetName(), *DependencyClass->GetName());
		}

		if (ProviderIndex == Index)
		{
			return FString::Printf(TEXT("'%s' depends on itself through '%s'. A Controller cannot be its own dependency."), *ControllerClass->GetName(), *DependencyClass->GetName());
		}

		if (ProviderIndex > Index)
		{
			return FString::Printf(TEXT("'%s' depends on '%s', which is listed after it. Move '%s' before it in UControllerSettings.Controllers."), *ControllerClass->GetName(), *DependencyClass->GetName(), *DependencyClass->GetName());
		}
	}

	return FString();
}

bool UControllerOrchestrator::InitializeControllers(UModelContainer* Model, const TArray<TSubclassOf<UControllerBase>>& ControllerClasses)
{
	if (!ensureMsgf(Model, TEXT("Controllers need a Model, but none was given.")))
	{
		return false;
	}

	const FString ValidationError = ValidateDependencyOrder(ControllerClasses);
	if (!ValidationError.IsEmpty())
	{
		UE_LOG(LogKeyCoreController, Error, TEXT("%s"), *ValidationError);
		ensureMsgf(false, TEXT("%s"), *ValidationError);
		return false;
	}

	FControllerContext Context;
	Context.Model = Model;

	for (const TSubclassOf<UControllerBase>& ControllerClass : ControllerClasses)
	{
		// Validation guarantees that every dependency is already in Controllers, because it is listed earlier.
		UControllerBase* Controller = NewObject<UControllerBase>(this, ControllerClass);
		DeliverDependencies(*Controller);
		Controller->Initialize(Context);
		Controllers.Add(Controller);
	}

	UE_LOG(LogKeyCoreController, Log, TEXT("Initialized %d Controllers in list order."), Controllers.Num());
	return true;
}

void UControllerOrchestrator::DeliverDependencies(UControllerBase& Controller)
{
	for (const TSubclassOf<UControllerBase>& DependencyClass : Controller.GetDependencies())
	{
		const TObjectPtr<UControllerBase>* InitializedDependency = Controllers.FindByPredicate([&DependencyClass](const UControllerBase* Initialized)
		{
			return Initialized->IsA(DependencyClass);
		});

		if (ensureMsgf(InitializedDependency, TEXT("'%s' has no initialized dependency '%s' although validation passed."), *Controller.GetClass()->GetName(), *DependencyClass->GetName()))
		{
			Controller.ReceiveDependency(*InitializedDependency);
		}
	}
}

void UControllerOrchestrator::DeinitializeControllers()
{
	for (int32 Index = Controllers.Num() - 1; Index >= 0; --Index)
	{
		Controllers[Index]->Deinitialize();
	}
	Controllers.Reset();
}

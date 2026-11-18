// Copyright Torben Borghoff, Keygen-Games

#include "Time/TimeViewModelResolver.h"

#include "Blueprint/UserWidget.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Model/KeyCoreModelProvider.h"
#include "Model/ModelContainer.h"
#include "Time/TimeModelComponent.h"
#include "Time/TimeViewModel.h"

UObject* UTimeViewModelResolver::CreateInstance(const UClass* ExpectedType, const UUserWidget* UserWidget, const UMVVMView* View) const
{
	if (!ExpectedType || !ExpectedType->IsChildOf(UTimeViewModel::StaticClass()))
	{
		return nullptr;
	}

	// The MVVM view keeps the instance alive and calls DestroyInstance, so the transient package is a valid outer here.
	//TODO claude: Verify the ViewModel lifetime under load (several widgets, many clients). Check that nothing collects it while the view is alive.
	UTimeViewModel* ViewModel = NewObject<UTimeViewModel>(GetTransientPackage());

	const UWorld* World = UserWidget ? UserWidget->GetWorld() : nullptr;
	const IKeyCoreModelProvider* ModelProvider = Cast<IKeyCoreModelProvider>(World ? World->GetGameState() : nullptr);
	UModelContainer* Model = ModelProvider ? ModelProvider->GetModel() : nullptr;
	if (!Model)
	{
		// Expected on a client that builds the widget before the GameState exists. The ViewModel then keeps its placeholder.
		UE_LOG(LogSimpleLifeSimTime, Warning, TEXT("TimeViewModelResolver: no Model available yet, the time widget shows a placeholder."));
		return ViewModel;
	}

	ViewModel->BindModel(Model->Get<UTimeModelComponent>());
	return ViewModel;
}

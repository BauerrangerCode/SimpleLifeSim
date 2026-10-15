// Copyright Torben Borghoff, Keygen-Games

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Controller/KeyCoreControllerOrchestrator.h"
#include "Model/ModelContainer.h"
#include "Tests/KeyCoreTestControllers.h"

BEGIN_DEFINE_SPEC(FKeyCoreControllerSpec, "KeyCore.Controller", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FKeyCoreControllerSpec)

void FKeyCoreControllerSpec::Define()
{
	Describe(TEXT("UControllerBase"), [this]()
	{
		It(TEXT("keeps the Model it was initialized with"), [this]()
		{
			UModelContainer* Model = NewObject<UModelContainer>(GetTransientPackage());
			UKeyCoreTestControllerA* Controller = NewObject<UKeyCoreTestControllerA>(GetTransientPackage());
			FControllerContext Context;
			Context.Model = Model;

			Controller->Initialize(Context);

			TestTrue(TEXT("The Controller reads the Model it was given"), Controller->GetModel() == Model);
		});

		It(TEXT("does not tick unless it opts in"), [this]()
		{
			UKeyCoreTestControllerA* Controller = NewObject<UKeyCoreTestControllerA>(GetTransientPackage());

			TestFalse(TEXT("Tick is opt-in and off by default"), Controller->CanTick());
		});
	});

	Describe(TEXT("UControllerOrchestrator"), [this]()
	{
		It(TEXT("initializes dependencies before their dependents and delivers them"), [this]()
		{
			UControllerOrchestrator* Orchestrator = NewObject<UControllerOrchestrator>(GetTransientPackage());
			UModelContainer* Model = NewObject<UModelContainer>(GetTransientPackage());
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerA::StaticClass(), UKeyCoreTestControllerB::StaticClass() };

			const bool bSucceeded = Orchestrator->InitializeControllers(Model, ControllerClasses);

			TestTrue(TEXT("A correctly ordered list initializes"), bSucceeded);
			TestEqual(TEXT("Both Controllers were created"), Orchestrator->GetControllers().Num(), 2);
			if (Orchestrator->GetControllers().Num() == 2)
			{
				const UKeyCoreTestControllerB* Dependent = Cast<UKeyCoreTestControllerB>(Orchestrator->GetControllers()[1]);
				TestNotNull(TEXT("The second Controller is the dependent"), Dependent);
				TestTrue(TEXT("The dependent received the initialized dependency"), Dependent && Dependent->ReceivedDependency == Orchestrator->GetControllers()[0]);
			}
		});

		It(TEXT("accepts a dependency that is listed before its dependent"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerA::StaticClass(), UKeyCoreTestControllerB::StaticClass() };

			TestTrue(TEXT("A correct order has no validation error"), UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses).IsEmpty());
		});

		It(TEXT("reports a dependent that is listed before its dependency"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerB::StaticClass(), UKeyCoreTestControllerA::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("A wrong order is reported as 'listed after it'"), Error.Contains(TEXT("is listed after it")));
		});

		It(TEXT("reports a missing dependency"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerB::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("A missing dependency is reported as missing"), Error.Contains(TEXT("is missing from UControllerSettings")));
		});

		It(TEXT("reports a Controller that depends on itself"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerSelfDependent::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("A self-dependency is reported"), Error.Contains(TEXT("depends on itself")));
		});

		It(TEXT("reports an empty dependency class"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerEmptyDependency::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("An empty dependency is reported"), Error.Contains(TEXT("returns an empty class")));
		});

		It(TEXT("reports an abstract Controller class"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UControllerBase::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("An abstract class is reported"), Error.Contains(TEXT("is abstract")));
		});

		It(TEXT("reports a Controller that is listed twice"), [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerA::StaticClass(), UKeyCoreTestControllerA::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue(TEXT("A duplicate is reported"), Error.Contains(TEXT("is listed twice")));
		});

		It(TEXT("does not tick before the world begins play"), [this]()
		{
			UControllerOrchestrator* Orchestrator = NewObject<UControllerOrchestrator>(GetTransientPackage());

			TestFalse(TEXT("The Orchestrator is inactive until the authority starts it"), Orchestrator->IsTickable());
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS

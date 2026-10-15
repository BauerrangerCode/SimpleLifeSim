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
	Describe("UControllerBase", [this]()
	{
		It("keeps the Model it was initialized with", [this]()
		{
			UModelContainer* Model = NewObject<UModelContainer>(GetTransientPackage());
			UKeyCoreTestControllerA* Controller = NewObject<UKeyCoreTestControllerA>(GetTransientPackage());
			FControllerContext Context;
			Context.Model = Model;

			Controller->Initialize(Context);

			TestTrue("The Controller reads the Model it was given", Controller->GetModel() == Model);
		});

		It("does not tick unless it opts in", [this]()
		{
			UKeyCoreTestControllerA* Controller = NewObject<UKeyCoreTestControllerA>(GetTransientPackage());

			TestFalse("Tick is opt-in and off by default", Controller->CanTick());
		});
	});

	Describe("UControllerOrchestrator", [this]()
	{
		It("initializes dependencies before their dependents and delivers them", [this]()
		{
			UControllerOrchestrator* Orchestrator = NewObject<UControllerOrchestrator>(GetTransientPackage());
			UModelContainer* Model = NewObject<UModelContainer>(GetTransientPackage());
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerA::StaticClass(), UKeyCoreTestControllerB::StaticClass() };

			const bool bSucceeded = Orchestrator->InitializeControllers(Model, ControllerClasses);

			TestTrue("A correctly ordered list initializes", bSucceeded);
			TestEqual("Both Controllers were created", Orchestrator->GetControllers().Num(), 2);
			if (Orchestrator->GetControllers().Num() == 2)
			{
				const UKeyCoreTestControllerB* Dependent = Cast<UKeyCoreTestControllerB>(Orchestrator->GetControllers()[1]);
				TestNotNull("The second Controller is the dependent", Dependent);
				TestTrue("The dependent received the initialized dependency", Dependent && Dependent->ReceivedDependency == Orchestrator->GetControllers()[0]);
			}
		});

		It("accepts a dependency that is listed before its dependent", [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerA::StaticClass(), UKeyCoreTestControllerB::StaticClass() };

			TestTrue("A correct order has no validation error", UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses).IsEmpty());
		});

		It("reports a dependent that is listed before its dependency", [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerB::StaticClass(), UKeyCoreTestControllerA::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue("A wrong order is reported as 'listed after it'", Error.Contains(TEXT("is listed after it")));
		});

		It("reports a missing dependency", [this]()
		{
			const TArray<TSubclassOf<UControllerBase>> ControllerClasses = { UKeyCoreTestControllerB::StaticClass() };

			const FString Error = UControllerOrchestrator::ValidateDependencyOrder(ControllerClasses);

			TestTrue("A missing dependency is reported as missing", Error.Contains(TEXT("is missing from UControllerSettings")));
		});

		It("does not tick before the world begins play", [this]()
		{
			UControllerOrchestrator* Orchestrator = NewObject<UControllerOrchestrator>(GetTransientPackage());

			TestFalse("The Orchestrator is inactive until the authority starts it", Orchestrator->IsTickable());
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS

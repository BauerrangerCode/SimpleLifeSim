// Copyright Torben Borghoff, Keygen-Games

#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS && WITH_EDITORONLY_DATA

#include "GameFramework/Actor.h"
#include "Model/ModelContainer.h"
#include "Tests/KeyCoreTestModelComponent.h"

BEGIN_DEFINE_SPEC(FKeyCoreModelSpec, "KeyCore.Model", EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter)
END_DEFINE_SPEC(FKeyCoreModelSpec)

void FKeyCoreModelSpec::Define()
{
	Describe(TEXT("UKeyCoreModelComponent"), [this]()
	{
		It(TEXT("is replicated by default"), [this]()
		{
			UKeyCoreTestModelComponent* Model = NewObject<UKeyCoreTestModelComponent>(GetTransientPackage());

			TestTrue(TEXT("The Sub-Model replicates by default"), Model->GetIsReplicated());
		});

		It(TEXT("notifies FieldNotify subscribers only when the value changes"), [this]()
		{
			UKeyCoreTestModelComponent* Model = NewObject<UKeyCoreTestModelComponent>(GetTransientPackage());
			const UE::FieldNotification::FFieldId FieldId = Model->GetFieldNotificationDescriptor().GetField(UKeyCoreTestModelComponent::StaticClass(), GET_MEMBER_NAME_CHECKED(UKeyCoreTestModelComponent, TestValue));
			TestTrue(TEXT("The FieldNotify descriptor finds TestValue"), FieldId.IsValid());

			int32 NotificationCount = 0;
			Model->AddFieldValueChangedDelegate(FieldId, INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateLambda(
				[&NotificationCount](UObject*, UE::FieldNotification::FFieldId)
				{
					++NotificationCount;
				}));

			Model->SetTestValue(5);
			Model->SetTestValue(5);
			Model->SetTestValue(7);

			TestEqual(TEXT("Only the two real changes notify"), NotificationCount, 2);
			TestEqual(TEXT("The value is stored"), Model->TestValue, 7);
		});
	});

	Describe(TEXT("UModelContainer"), [this]()
	{
		It(TEXT("resolves a Sub-Model by its class"), [this]()
		{
			AActor* Owner = NewObject<AActor>(GetTransientPackage());
			UKeyCoreTestModelComponent* Model = NewObject<UKeyCoreTestModelComponent>(Owner);
			Owner->AddInstanceComponent(Model);
			UModelContainer* Container = NewObject<UModelContainer>(Owner);

			TestTrue(TEXT("Get<T> returns the Sub-Model owned by the GameState"), Container->Get<UKeyCoreTestModelComponent>() == Model);
		});

		It(TEXT("returns the same instance on repeated calls"), [this]()
		{
			AActor* Owner = NewObject<AActor>(GetTransientPackage());
			UKeyCoreTestModelComponent* Model = NewObject<UKeyCoreTestModelComponent>(Owner);
			Owner->AddInstanceComponent(Model);
			UModelContainer* Container = NewObject<UModelContainer>(Owner);

			UKeyCoreTestModelComponent* First = Container->Get<UKeyCoreTestModelComponent>();
			UKeyCoreTestModelComponent* Second = Container->Get<UKeyCoreTestModelComponent>();

			TestTrue(TEXT("The cached Sub-Model is returned unchanged"), First == Second && First == Model);
		});
	});
}

#endif // WITH_DEV_AUTOMATION_TESTS

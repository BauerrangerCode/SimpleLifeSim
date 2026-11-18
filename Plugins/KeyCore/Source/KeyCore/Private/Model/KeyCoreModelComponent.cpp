// Copyright Torben Borghoff, Keygen-Games

#include "Model/KeyCoreModelComponent.h"

#include "Net/UnrealNetwork.h"

UKeyCoreModelComponent::UKeyCoreModelComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	// Sub-Models are replicated to every client by default, because clients read them for their UI.
	SetIsReplicatedByDefault(true);
}

void UKeyCoreModelComponent::FFieldNotificationClassDescriptor::ForEachField(const UClass* Class, TFunctionRef<bool(::UE::FieldNotification::FFieldId FielId)> Callback) const
{
	// The base has no fields. Subclasses chain to this descriptor and add their own fields first.
}

const UE::FieldNotification::IClassDescriptor& UKeyCoreModelComponent::GetFieldNotificationDescriptor() const
{
	static FFieldNotificationClassDescriptor Local;
	return Local;
}

FDelegateHandle UKeyCoreModelComponent::AddFieldValueChangedDelegate(UE::FieldNotification::FFieldId InFieldId, FFieldValueChangedDelegate InNewDelegate)
{
	return NotificationDelegates.AddFieldValueChangedDelegate(this, InFieldId, MoveTemp(InNewDelegate));
}

bool UKeyCoreModelComponent::RemoveFieldValueChangedDelegate(UE::FieldNotification::FFieldId InFieldId, FDelegateHandle InHandle)
{
	return NotificationDelegates.RemoveFieldValueChangedDelegate(this, InFieldId, InHandle);
}

int32 UKeyCoreModelComponent::RemoveAllFieldValueChangedDelegates(FDelegateUserObjectConst InUserObject)
{
	return NotificationDelegates.RemoveAllFieldValueChangedDelegates(this, InUserObject);
}

int32 UKeyCoreModelComponent::RemoveAllFieldValueChangedDelegates(UE::FieldNotification::FFieldId InFieldId, FDelegateUserObjectConst InUserObject)
{
	return NotificationDelegates.RemoveAllFieldValueChangedDelegates(this, InFieldId, InUserObject);
}

void UKeyCoreModelComponent::BroadcastFieldValueChanged(UE::FieldNotification::FFieldId InFieldId)
{
	NotificationDelegates.BroadcastFieldValueChanged(this, InFieldId);
}

void UKeyCoreModelComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
}

// Copyright Torben Borghoff, Keygen-Games

#include "Time/TimeModelComponent.h"

#include "MVVMViewModelBase.h"
#include "Net/UnrealNetwork.h"

DEFINE_LOG_CATEGORY(LogSimpleLifeSimTime);

UTimeModelComponent::UTimeModelComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UTimeModelComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UTimeModelComponent, MinuteOfDay);
}

int32 UTimeModelComponent::GetMinuteOfDay() const
{
	return MinuteOfDay;
}

void UTimeModelComponent::SetMinuteOfDay(int32 NewMinuteOfDay)
{
	SetPropertyValue(MinuteOfDay, NewMinuteOfDay, ThisClass::FFieldNotificationClassDescriptor::MinuteOfDay);
}

void UTimeModelComponent::OnRep_MinuteOfDay()
{
	UE_LOG(LogSimpleLifeSimTime, Verbose, TEXT("MinuteOfDay replicated to a client: %d."), MinuteOfDay);
	UE_MVVM_BROADCAST_FIELD_VALUE_CHANGED(MinuteOfDay);
}

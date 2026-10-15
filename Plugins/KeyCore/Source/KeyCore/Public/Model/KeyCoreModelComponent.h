// Copyright Torben Borghoff, Keygen-Games

#pragma once

#include "Components/GameStateComponent.h"
#include "IFieldNotificationClassDescriptor.h"
#include "INotifyFieldValueChanged.h"
#include "ViewModel/MVVMFieldNotificationDelegates.h"

#include "KeyCoreModelComponent.generated.h"

/**
 * Base of every Sub-Model on the GameState. It holds replicated game data so that every client can read it,
 * and it notifies ViewModels through FieldNotify. Only Controllers may write to it. This is a convention checked
 * in review, not enforced by the type system, because the project chose not to add friend or passkey restrictions.
 */
UCLASS(Abstract, MinimalAPI)
class UKeyCoreModelComponent : public UGameStateComponent, public INotifyFieldValueChanged
{
	GENERATED_BODY()

public:
	/**
	 * Base of the FieldNotify descriptor. Subclasses with FieldNotify properties extend it through the
	 * UHT-generated descriptor macros. This class has no fields of its own.
	 */
	struct FFieldNotificationClassDescriptor : public ::UE::FieldNotification::IClassDescriptor
	{
		KEYCORE_API virtual void ForEachField(const UClass* Class, TFunctionRef<bool(::UE::FieldNotification::FFieldId FielId)> Callback) const override;
	};

public:
	UKeyCoreModelComponent(const FObjectInitializer& ObjectInitializer);

	//~ Begin INotifyFieldValueChanged Interface
	KEYCORE_API virtual FDelegateHandle AddFieldValueChangedDelegate(UE::FieldNotification::FFieldId InFieldId, FFieldValueChangedDelegate InNewDelegate) override final;
	KEYCORE_API virtual bool RemoveFieldValueChangedDelegate(UE::FieldNotification::FFieldId InFieldId, FDelegateHandle InHandle) override final;
	KEYCORE_API virtual int32 RemoveAllFieldValueChangedDelegates(FDelegateUserObjectConst InUserObject) override final;
	KEYCORE_API virtual int32 RemoveAllFieldValueChangedDelegates(UE::FieldNotification::FFieldId InFieldId, FDelegateUserObjectConst InUserObject) override final;
	KEYCORE_API virtual void BroadcastFieldValueChanged(UE::FieldNotification::FFieldId InFieldId) override final;
	KEYCORE_API virtual const UE::FieldNotification::IClassDescriptor& GetFieldNotificationDescriptor() const override;
	//~ End INotifyFieldValueChanged Interface

	/** Replicated properties of the Model. Subclasses register their own properties and call Super first. */
	KEYCORE_API virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	/**
	 * Stores the value and notifies FieldNotify subscribers only if it changed. Writing the same value again must not
	 * broadcast, otherwise ViewModels refresh for nothing. Mirrors UMVVMViewModelBase::SetPropertyValue.
	 */
	template<typename T, typename U = T>
	bool SetPropertyValue(T& Value, const U& NewValue, UE::FieldNotification::FFieldId FieldId)
	{
		if (Value == NewValue)
		{
			return false;
		}

		Value = NewValue;
		BroadcastFieldValueChanged(FieldId);
		return true;
	}

private:
	/** Per-field delegates. The MVVM plugin's helper is used so FieldNotify behaves exactly like on ViewModels. */
	UE::MVVM::FMVVMFieldNotificationDelegates NotificationDelegates;
};

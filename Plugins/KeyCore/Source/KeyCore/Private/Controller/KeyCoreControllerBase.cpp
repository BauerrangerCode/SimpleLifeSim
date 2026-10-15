// Copyright Torben Borghoff, Keygen-Games

#include "Controller/KeyCoreControllerBase.h"

#include "Model/ModelContainer.h"

void UControllerBase::Initialize(const FControllerContext& Context)
{
	Model = Context.Model;
}

void UControllerBase::Deinitialize()
{
	Model = nullptr;
}

void UControllerBase::Tick(float DeltaSeconds)
{
}

TArray<TSubclassOf<UControllerBase>> UControllerBase::GetDependencies() const
{
	return {};
}

void UControllerBase::ReceiveDependency(UControllerBase* Dependency)
{
}

UModelContainer* UControllerBase::GetModel() const
{
	return Model;
}

bool UControllerBase::CanTick() const
{
	return bCanTick;
}

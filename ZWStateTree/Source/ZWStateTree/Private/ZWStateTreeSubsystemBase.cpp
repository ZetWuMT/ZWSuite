// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWStateTreeSubsystemBase.h"

#include "StateTree.h"
#include "StateTreeExecutionContext.h"

void UZWStateTreeSubsystemBase::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UStateTree* TreeAsset = GetStateTreeAsset();
	if (!TreeAsset)
	{
		return;
	}

	StateTreeRef.SetStateTree(const_cast<UStateTree*>(TreeAsset));
	StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());
}

void UZWStateTreeSubsystemBase::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);

	// Only (re)start the tree once the local player actually owns a controller - this mirrors
	// the behavior of the original, hand-written ZWInputSubsystem / ZWUIStateTreeSubsystem.
	if (NewPlayerController)
	{
		const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
		if (TreeAsset && StateTreeInstanceData.Num() > 0)
		{
			FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
			BindContextData(Context, TreeAsset);

			Context.Start();
		}
	}
	// else: optionally Context.Stop() here in a derived class if you need to react to the
	// local player losing its controller - the base class intentionally does nothing here,
	// same as the original implementations did.
}

void UZWStateTreeSubsystemBase::Tick(float DeltaTime)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();

	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);

		Context.Tick(DeltaTime);
	}
}

TStatId UZWStateTreeSubsystemBase::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZWStateTreeSubsystemBase, STATGROUP_Tickables);
}

bool UZWStateTreeSubsystemBase::IsTickable() const
{
	// Prevent ticking the Class Default Object in the editor.
	return !HasAnyFlags(RF_ClassDefaultObject);
}

void UZWStateTreeSubsystemBase::StartStateTree()
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	if (!TreeAsset)
	{
		return;
	}

	// Copy default data layout from the asset (matches the old StartUITree() behavior).
	StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());

	FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
	BindContextData(Context, TreeAsset);

	Context.Start();
}

void UZWStateTreeSubsystemBase::SendStateTreeEvent(FGameplayTag EventTag)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();

	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);

		Context.SendEvent(EventTag);
	}
}

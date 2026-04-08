// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWUIStateTreeSubsystem.h"

#include "StateTreeExecutionContext.h"
#include "ZWUIStateTreeSettings.h"
#include "ZWUISubsystem.h" // We need this to bind it to the context
// #include "ZWUIStateTreeSettings.h" // Assuming you will create settings for this plugin too!

void UZWUIStateTreeSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	const UZWUIStateTreeSettings* Settings = GetDefault<UZWUIStateTreeSettings>();
	if (Settings && !Settings->DefaultUIStateTree.IsNull())
	{
		StateTreeRef.SetStateTree(Settings->DefaultUIStateTree.LoadSynchronous());
	}
	
	if (!StateTreeRef.IsValid()) return;

	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	if (!TreeAsset) return;
	
	StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());
}

void UZWUIStateTreeSubsystem::PlayerControllerChanged(APlayerController* NewPlayerController)
{
	Super::PlayerControllerChanged(NewPlayerController);
	
	// Odpalamy drzewo tylko wtedy, gdy gracz faktycznie DOSTAJE kontroler
	if (NewPlayerController)
	{
		const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
		if (TreeAsset && StateTreeInstanceData.Num() > 0)
		{
			FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
			BindContextData(Context, TreeAsset);

			Context.Start(); 
		}
		
		if (ULocalPlayer* LP = GetLocalPlayer())
		{
			UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>();

			if (UISubsystem)
			{
				UISubsystem->OnGameplayTagSent.AddUObject(this, &UZWUIStateTreeSubsystem::ProcessUITag);
			}
		}
	}
	else
	{
		// Opcjonalnie: Gdy gracz traci kontroler, moglibyśmy tu wywołać Context.Stop(),
		// żeby drzewo przestało nasłuchiwać i zresetowało swoje stany.
	}
}

bool UZWUIStateTreeSubsystem::IsTickable() const
{
	// Prevent ticking the Class Default Object in the editor
	return !HasAnyFlags(RF_ClassDefaultObject);
}

TStatId UZWUIStateTreeSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UZWUIStateTreeSubsystem, STATGROUP_Tickables);
}

void UZWUIStateTreeSubsystem::Tick(float DeltaTime)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	
	// Tick the tree if it is loaded and initialized
	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);
		Context.Tick(DeltaTime);
	}
}

void UZWUIStateTreeSubsystem::BindContextData(FStateTreeExecutionContext& Context, const UStateTree* TreeAsset)
{
	if (!TreeAsset) return;

	// Retrieve the context descriptors built by the compiler
	TConstArrayView<FStateTreeExternalDataDesc> ContextDescs = TreeAsset->GetContextDataDescs();

	// Inject the ZWUISubsystem required by our ZWUIStateTreeSchema
	for (const FStateTreeExternalDataDesc& Desc : ContextDescs)
	{
		if (Desc.Struct && Desc.Struct->IsChildOf(UZWUISubsystem::StaticClass()))
		{
			// Fetch the UI subsystem from the local player
			if (ULocalPlayer* LP = GetLocalPlayer())
			{
				if (UZWUISubsystem* UISubsystem = LP->GetSubsystem<UZWUISubsystem>())
				{
					FStateTreeDataView UIView(UISubsystem);
					Context.SetContextData(Desc.Handle, UIView);
				}
			}
			break; 
		}
	}
}

void UZWUIStateTreeSubsystem::StartUITree()
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	if (TreeAsset)
	{
		// Copy default data layout from the asset
		StateTreeInstanceData.CopyFrom(*this, TreeAsset->GetDefaultInstanceData());

		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);
		
		Context.Start();
	}
}

void UZWUIStateTreeSubsystem::ProcessUITag(FGameplayTag UITag)
{
	const UStateTree* TreeAsset = StateTreeRef.GetStateTree();
	
	//UZWUISubsystem* UISubsystem = GetLocalPlayer()->GetSubsystem<UZWUISubsystem>();
	//if (!UISubsystem || !UISubsystem->IsPanelRegisteredByTag(UITag)) return;
	
	if (TreeAsset && StateTreeInstanceData.Num() > 0)
	{
		FStateTreeExecutionContext Context(*this, *TreeAsset, StateTreeInstanceData);
		BindContextData(Context, TreeAsset);
		
		Context.SendEvent(UITag);
	}
}

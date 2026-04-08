// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUIStateTreeSchema.h"

#include "StateTreeConditionBase.h"
#include "StateTreeTaskBase.h"
#include "StateTreeTypes.h"
#include "ZWUISubsystem.h"

UZWUIStateTreeSchema::UZWUIStateTreeSchema()
{
	// Define that EVERY tree of this type MUST receive ZWUISubsystem to function
	FStateTreeExternalDataDesc UISubsystemDesc;
	UISubsystemDesc.Name = TEXT("UISubsystem");
	UISubsystemDesc.Struct = UZWUISubsystem::StaticClass();
	UISubsystemDesc.Requirement = EStateTreeExternalDataRequirement::Required;
	
	ContextDataDescs.Add(UISubsystemDesc);
}

bool UZWUIStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	// Allow using standard StateTree Tasks and Conditions
	return InScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct());
}

bool UZWUIStateTreeSchema::IsClassAllowed(const UClass* InClass) const
{
	if (InClass && InClass->IsChildOf(UZWUISubsystem::StaticClass()))
	{
		return true;
	}

	return Super::IsClassAllowed(InClass);
}

bool UZWUIStateTreeSchema::IsExternalItemAllowed(const UStruct& InStruct) const
{
	if (InStruct.IsChildOf(UZWUISubsystem::StaticClass()))
	{
		return true;
	}

	return Super::IsExternalItemAllowed(InStruct);
}

TConstArrayView<FStateTreeExternalDataDesc> UZWUIStateTreeSchema::GetContextDataDescs() const
{
	return ContextDataDescs;
}

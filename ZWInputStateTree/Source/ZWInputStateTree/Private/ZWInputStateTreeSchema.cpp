// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInputStateTreeSchema.h"
#include "ZWInputSubsystem.h"
#include "StateTreeTaskBase.h"
#include "StateTreeConditionBase.h"
#include "StateTreeEvaluatorBase.h"
#include "StateTreeTypes.h"
#include "GameFramework/Actor.h"

UZWInputStateTreeSchema::UZWInputStateTreeSchema()
{
	ContextDataDescs.Add(FStateTreeExternalDataDesc(
		FName("ZWInputSubsystem"), 
		UZWInputSubsystem::StaticClass(), 
		FGuid(0x11111111, 0x22222222, 0x33333333, 0x44444444) 
	));

	ContextDataDescs.Add(FStateTreeExternalDataDesc(
		FName("PlayerActor"), 
		AActor::StaticClass(), 
		FGuid(0x55555555, 0x66666666, 0x77777777, 0x88888888) 
	));
	
	ContextDataDescs.Add(FStateTreeExternalDataDesc(
	FName("PlayerController"), 
	APlayerController::StaticClass(), 
	FGuid(0x22222222, 0x44444444, 0x66666666, 0x88888888) 
));

}

bool UZWInputStateTreeSchema::IsStructAllowed(const UScriptStruct* InScriptStruct) const
{
	// Allow the base StateTree structures
	return InScriptStruct->IsChildOf(FStateTreeTaskBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeConditionBase::StaticStruct()) ||
		   InScriptStruct->IsChildOf(FStateTreeEvaluatorBase::StaticStruct());
}

bool UZWInputStateTreeSchema::IsClassAllowed(const UClass* InClass) const
{
	// 1. Always allow if the class is empty (required by the engine for pin clearing)
	if (!InClass)
	{
		return false; 
	}

	// 2. Pass our Subsystem
	if (InClass->IsChildOf(UZWInputSubsystem::StaticClass()))
	{
		return true;
	}

	// 3. ALLOW THE ACTOR! (This fixes your bug)
	if (InClass->IsChildOf(AActor::StaticClass()))
	{
		return true;
	}
	
	if (InClass->IsChildOf(APlayerController::StaticClass()))
	{
		return true;
	}

	// 4. If it is none of the above, ask the base class
	return Super::IsClassAllowed(InClass);
}
// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInventoryItemDefinitionFactory.h"
#include "ZWInventoryItemDefinition.h"

UZWInventoryItemDefinitionFactory::UZWInventoryItemDefinitionFactory()
{
	SupportedClass = UZWInventoryItemDefinition::StaticClass();

	bCreateNew = true;

	bEditAfterNew = true;
}

UObject* UZWInventoryItemDefinitionFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName,
                                                           EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UZWInventoryItemDefinition>(InParent, InClass, InName, Flags | RF_Transactional);
}

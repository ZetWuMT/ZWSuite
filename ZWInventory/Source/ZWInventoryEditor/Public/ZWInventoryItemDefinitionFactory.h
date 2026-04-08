// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Factories/Factory.h"
#include "ZWInventoryItemDefinitionFactory.generated.h"

/**
 * 
 */
UCLASS()
class ZWINVENTORYEDITOR_API UZWInventoryItemDefinitionFactory : public UFactory
{
	GENERATED_BODY()

public:
	UZWInventoryItemDefinitionFactory();

	
	virtual UObject* FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

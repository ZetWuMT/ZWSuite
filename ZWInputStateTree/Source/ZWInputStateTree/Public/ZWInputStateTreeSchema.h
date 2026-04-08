// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeExecutionTypes.h"
#include "StateTreeSchema.h"
#include "ZWInputStateTreeSchema.generated.h"

/**
 * 
 */
UCLASS()
class ZWINPUTSTATETREE_API UZWInputStateTreeSchema : public UStateTreeSchema
{
	GENERATED_BODY()
	
public:
	UZWInputStateTreeSchema();
	
	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override { return ContextDataDescs; }

protected:
	// Definiujemy, jakie klasy są wymagane jako dane zewnętrzne dla drzewa
	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
	virtual bool IsClassAllowed(const UClass* InClass) const override;
	
	UPROPERTY()
	TArray<FStateTreeExternalDataDesc> ContextDataDescs;
};

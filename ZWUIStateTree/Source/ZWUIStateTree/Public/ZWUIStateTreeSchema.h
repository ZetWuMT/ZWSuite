// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StateTreeSchema.h"

#include "ZWUIStateTreeSchema.generated.h"

/**
 * Schema dedicated to User Interface (UI) management.
 * Requires UZWUISubsystem to be provided as context data.
 */
UCLASS(BlueprintType, EditInlineNew, CollapseCategories, meta = (DisplayName = "ZWUI State Tree"))
class ZWUISTATETREE_API UZWUIStateTreeSchema : public UStateTreeSchema
{
	GENERATED_BODY()
	
public:
	UZWUIStateTreeSchema();

	// Filtrujemy, jakie Taski/Warunki mogą być używane w tym drzewie
	virtual bool IsStructAllowed(const UScriptStruct* InScriptStruct) const override;
	virtual bool IsClassAllowed(const UClass* InClass) const override;
	virtual bool IsExternalItemAllowed(const UStruct& InStruct) const override;

	// Wystawiamy nasze wymagania na zewnątrz
	virtual TConstArrayView<FStateTreeExternalDataDesc> GetContextDataDescs() const override;

protected:
	UPROPERTY()
	TArray<FStateTreeExternalDataDesc> ContextDataDescs;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ZWInteractionInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UZWInteractionInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 BYPASSED FOR NOW
 */
class ZWINTERACTION_API IZWInteractionInterface
{
	GENERATED_BODY()

public:
	virtual void Interact();

	virtual void Inspect();

	virtual void Investigate();

	virtual bool IsInspectable();

	virtual bool IsInvestigatable();

	virtual bool IsInvestigationExclusive();

	virtual void ToggleHighlight(bool isHighlighted);

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Interaction System")
	void BPToggleHighlight(bool isHighlighted);
};


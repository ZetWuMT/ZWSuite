// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/Overlay.h"
#include "ZWUIHUDElementSlot.generated.h"

class UZWUIHUDElement;
/**
 * 
 */
UCLASS()
class ZWUIHUD_API UZWUIHUDElementSlot : public UOverlay
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ZWUIHUDElement")
	void InitializeHUDSlot();
	
protected:
	UPROPERTY(EditAnywhere, Category = "ZWUIHUDElement")
	TSoftClassPtr<UZWUIHUDElement> HUDElementClass;

	UPROPERTY(Transient)
	TObjectPtr<UZWUIHUDElement> SpawnedHUDElement;

	UPROPERTY(EditAnywhere, Category = "ZWUIHUDElement")
	FMargin SlotPadding;

	UPROPERTY(EditAnywhere, Category = "ZWUIHUDElement")
	TEnumAsByte<EHorizontalAlignment> SlotHorizontalAlignment;

	UPROPERTY(EditAnywhere, Category = "ZWUIHUDElement")
	TEnumAsByte<EVerticalAlignment> SlotVerticalAlignment;
	
	UPROPERTY()
	bool bHasSpawnedHUDElement = false;
};

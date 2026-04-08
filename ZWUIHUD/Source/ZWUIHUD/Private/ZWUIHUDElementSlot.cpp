// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUIHUDElementSlot.h"

#include "ZWUIHUDElement.h"
#include "Components/OverlaySlot.h"

void UZWUIHUDElementSlot::InitializeHUDSlot()
{
	if (bHasSpawnedHUDElement || SpawnedHUDElement) return;
	
	if (HUDElementClass.IsNull()) return;
	
	UClass* LoadedClass = HUDElementClass.LoadSynchronous();
	if (!LoadedClass) return;

	SpawnedHUDElement = CreateWidget<UZWUIHUDElement>(GetWorld(), LoadedClass);
	
	if(SpawnedHUDElement)
	{
		if (UOverlaySlot* OverlaySlot = AddChildToOverlay(SpawnedHUDElement))
		{
			OverlaySlot->SetHorizontalAlignment(SlotHorizontalAlignment);
			OverlaySlot->SetVerticalAlignment(SlotVerticalAlignment);
			OverlaySlot->SetPadding(SlotPadding);
		}

		bHasSpawnedHUDElement = true;
	}
}
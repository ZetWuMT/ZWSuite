// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWUIHUDPanel.h"

#include "ZWUIHUDElementSlot.h"
#include "Blueprint/WidgetTree.h"

void UZWUIHUDPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (WidgetTree)
	{
		// Iterate through every single widget placed into this HUD in the editor
		WidgetTree->ForEachWidget([&](UWidget* FoundWidget)
		{
			// Check whether the found widget is our special Slot
			if (UZWUIHUDElementSlot* HUDSlot = Cast<UZWUIHUDElementSlot>(FoundWidget))
			{
				// Found it! Fire the initialization logic
				HUDSlot->InitializeHUDSlot();
				
				// Optional: log to be sure it finds the slots
				// UE_LOG(LogTemp, Log, TEXT("Found and initialized slot: %s"), *HUDSlot->GetName());
			}
		});
	}
}

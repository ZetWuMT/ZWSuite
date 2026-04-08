// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWUIHUDPanel.h"

#include "ZWUIHUDElementSlot.h"
#include "Blueprint/WidgetTree.h"

void UZWUIHUDPanel::NativeConstruct()
{
	Super::NativeConstruct();
	
	if (WidgetTree)
	{
		// Przeszukujemy każdy pojedynczy widget wrzucony do tego HUDa w edytorze
		WidgetTree->ForEachWidget([&](UWidget* FoundWidget)
		{
			// Sprawdzamy, czy znaleziony widget to nasz specjalny Slot
			if (UZWUIHUDElementSlot* HUDSlot = Cast<UZWUIHUDElementSlot>(FoundWidget))
			{
				// Znaleźliśmy! Odpalamy logikę inicjalizacji
				HUDSlot->InitializeHUDSlot();
				
				// Opcjonalnie: log dla pewności, że znajduje sloty
				// UE_LOG(LogTemp, Log, TEXT("Znalazlem i zainicjalizowalem slot: %s"), *HUDSlot->GetName());
			}
		});
	}
}

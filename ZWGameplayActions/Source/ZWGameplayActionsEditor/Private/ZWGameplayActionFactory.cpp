#include "ZWGameplayActionFactory.h"

#if WITH_EDITOR

#include "ZWGameplayAction.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "BlueprintEditorSettings.h"
#include "Kismet2/BlueprintEditorUtils.h"

UZWGameplayActionFactory::UZWGameplayActionFactory()
{
	SupportedClass = UZWGameplayAction::StaticClass();
	ParentClass = UZWGameplayAction::StaticClass();
	bSkipClassPicker = true;
	
	bCreateNew = true;
	bEditAfterNew = true;
	
	bEditorImport = false;
}

FText UZWGameplayActionFactory::GetDisplayName() const
{
	// To jest nazwa, która pojawi się w menu kontekstowym pod prawym przyciskiem myszy!
	return FText::FromString("ZW Gameplay Action");
}

FString UZWGameplayActionFactory::GetDefaultNewAssetName() const
{
	// To jest domyślna nazwa pliku, gdy tylko go stworzysz (zanim wpiszesz własną)
	return FString("NewGameplayAction");
}

UObject* UZWGameplayActionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Pozwalamy bazowej fabryce stworzyć czysty Blueprint
	UBlueprint* NewBP = Cast<UBlueprint>(Super::FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn));

	if (NewBP)
	{
		// ZNAJDUJEMY EVENT GRAPH NOWEGO BLUEPRINTA
		UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(NewBP);
		if (EventGraph)
		{
			int32 NodePosY = 0;
			
			// MAGIA: Wrzucamy Twój czerwony event ExecuteAction na środek grafu!
			// Będzie on wyszarzony, dopóki gracz nie podepnie pod niego kabelka.
			FKismetEditorUtilities::AddDefaultEventNode(
				NewBP, 
				EventGraph, 
				FName(TEXT("ExecuteAction")), // Nazwa Twojej funkcji z C++
				UZWGameplayAction::StaticClass(), 
				NodePosY
			);
		}
	}

	return NewBP;
}

#endif
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
	// This is the name that will appear in the context menu under the right mouse button!
	return FText::FromString("ZW Gameplay Action");
}

FString UZWGameplayActionFactory::GetDefaultNewAssetName() const
{
	// This is the default file name when you create it (before you type your own)
	return FString("NewGameplayAction");
}

UObject* UZWGameplayActionFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// Let the base factory create a clean Blueprint
	UBlueprint* NewBP = Cast<UBlueprint>(Super::FactoryCreateNew(Class, InParent, Name, Flags, Context, Warn));

	if (NewBP)
	{
		// WE FIND THE EVENT GRAPH OF THE NEW BLUEPRINT
		UEdGraph* EventGraph = FBlueprintEditorUtils::FindEventGraph(NewBP);
		if (EventGraph)
		{
			int32 NodePosY = 0;
			
			// MAGIC: We drop your red ExecuteAction event into the middle of the graph!
			// It will be grayed out until the player hooks a wire into it.
			FKismetEditorUtilities::AddDefaultEventNode(
				NewBP, 
				EventGraph, 
				FName(TEXT("ExecuteAction")), // The name of your C++ function
				UZWGameplayAction::StaticClass(), 
				NodePosY
			);
		}
	}

	return NewBP;
}

#endif
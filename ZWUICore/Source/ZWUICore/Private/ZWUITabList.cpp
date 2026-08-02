// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUITabList.h"
#include "Components/PanelWidget.h"
#include "CommonButtonBase.h"

void UZWUITabList::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (TabButtonContainer && TabButton)
	{
		// Common UI just generated a button from the Data Asset - put it into our box!
		TabButtonContainer->AddChild(TabButton);
	}
}

void UZWUITabList::HandleTabRemoval_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (TabButtonContainer && TabButton)
	{
		TabButtonContainer->RemoveChild(TabButton);
	}
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUITabList.h"
#include "Components/PanelWidget.h"
#include "CommonButtonBase.h"

void UZWUITabList::HandleTabCreation_Implementation(FName TabNameID, UCommonButtonBase* TabButton)
{
	if (TabButtonContainer && TabButton)
	{
		// Common UI właśnie wygenerował przycisk z Data Assetu - wrzucamy go do naszego pudełka!
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

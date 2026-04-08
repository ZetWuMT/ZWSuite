// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWUITabListButton.h"

#include "CommonTextBlock.h"
#include "Components/Image.h"

void UZWUITabListButton::SetTabData(FText ButtonText, UTexture2D* ButtonIcon)
{
	TabButtonText->SetText(ButtonText);
	
	if (ButtonIcon && TabButtonImage)
	{
		TabButtonImage->SetBrushFromSoftTexture(ButtonIcon);
	}
}
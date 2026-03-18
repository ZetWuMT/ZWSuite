// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionInterface.h"

// Add default functionality here for any IZWInteractionInterface functions that are not pure virtual.

void IZWInteractionInterface::Interact()
{
}

void IZWInteractionInterface::Inspect()
{
}

void IZWInteractionInterface::Investigate()
{
}

void IZWInteractionInterface::ToggleHighlight(bool isHighlighted)
{
}

bool IZWInteractionInterface::IsInspectable()
{
	return false;
}

bool IZWInteractionInterface::IsInvestigatable()
{
	return false;
}

bool IZWInteractionInterface::IsInvestigationExclusive()
{
	return false;
}

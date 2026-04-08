// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWGameplayAction.h"

void UZWGameplayAction::ExecuteAction_Implementation(APlayerController* Controller, APawn* Pawn)
{
	// Do nothing by default
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("ExecuteAction!!"));
}

UWorld* UZWGameplayAction::GetWorld() const
{
	if (HasAnyFlags(RF_ClassDefaultObject)) return nullptr;
	return GetOuter() ? GetOuter()->GetWorld() : nullptr;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWAbilitySystemComponent.h"
#include "ZWInputComponent.h"

void UZWAbilitySystemComponent::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	
	TWeakObjectPtr<APlayerController> PlayerController = AbilityActorInfo->PlayerController;
	
	if (PlayerController.IsValid())
	{
		if (UZWInputComponent* InputComponent = Cast<UZWInputComponent>(PlayerController->InputComponent))
		{
			InputComponent->OnInputTagTriggered.RemoveAll(this);
		}
	}
}

void UZWAbilitySystemComponent::OnPlayerControllerSet()
{
	Super::OnPlayerControllerSet();
	
	TWeakObjectPtr<APlayerController> PlayerController = AbilityActorInfo->PlayerController;
	
	if (PlayerController.IsValid())
	{
		UZWInputComponent* InputComponent = PlayerController.Get()->GetComponentByClass<UZWInputComponent>();
		
		if (InputComponent)
		{
			InputComponent->OnInputTagSimpleTriggered.AddUObject(this, &UZWAbilitySystemComponent::HandleInputTag);
		}
	}
}

void UZWAbilitySystemComponent::HandleInputTag(FGameplayTag InGameplayTag)
{
	TryActivateAbilitiesByTag(FGameplayTagContainer(InGameplayTag));
}

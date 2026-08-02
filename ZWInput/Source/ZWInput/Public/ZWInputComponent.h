// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "ZWInputConfig.h"
#include "ZWInputComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FZWInputTagEvent, FGameplayTag /* InputTag*/, const FInputActionValue& /* InputActionValue*/);
DECLARE_MULTICAST_DELEGATE_OneParam(FZWInputTagSimpleEvent, FGameplayTag /* InputTag*/);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ZWINPUT_API UZWInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UZWInputComponent();
	
	void InitializeInput();
	
	template<class UserClass, typename FuncType>
	void BindNativeAction(FGameplayTag InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func);
	
	FZWInputTagEvent OnInputTagTriggered;
	FZWInputTagSimpleEvent OnInputTagSimpleTriggered;

private:
	void HandleGenericInput(const FInputActionValue& ActionValue, FGameplayTag InputTag);
	
	const UZWInputConfig* GetInputConfigAsset();
	
	UPROPERTY()
	const UZWInputConfig* CachedConfig = nullptr;
};

template<class UserClass, typename FuncType>
void UZWInputComponent::BindNativeAction(FGameplayTag InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func)
{
	if (!CachedConfig)
	{
		CachedConfig = GetInputConfigAsset();
	}
	
	if (!CachedConfig) return;

	// Look in our DataAsset for the action assigned to this specific Tag
	for (const FZWInputAction& Action : CachedConfig->NativeInputActions)
	{
		if (Action.InputTag.MatchesTagExact(InputTag))
		{
			if (UInputAction* IA = Action.InputAction.LoadSynchronous())
			{
				// Bind directly to the provided function
				BindAction(IA, TriggerEvent, Object, Func);
			}
		}
	}
}
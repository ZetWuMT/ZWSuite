// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "GameplayTagContainer.h"
#include "ZWInputConfig.h"
#include "ZWInputComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FZWInputTagEvent, FGameplayTag /* InputTag*/, const FInputActionValue& /* InputActionValue*/);

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

	// Szukamy w naszym DataAssetcie akcji przypisanej do tego konkretnego Taga
	for (const FZWInputAction& Action : CachedConfig->NativeInputActions)
	{
		if (Action.InputTag.MatchesTagExact(InputTag))
		{
			if (UInputAction* IA = Action.InputAction.LoadSynchronous())
			{
				// Bindujemy bezpośrednio do podanej funkcji
				BindAction(IA, TriggerEvent, Object, Func);
			}
		}
	}
}
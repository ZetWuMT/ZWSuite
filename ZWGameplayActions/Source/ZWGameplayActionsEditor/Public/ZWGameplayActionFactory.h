#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR

#include "Factories/BlueprintFactory.h"
#include "ZWGameplayActionFactory.generated.h"

UCLASS()
class ZWGAMEPLAYACTIONS_API UZWGameplayActionFactory : public UBlueprintFactory
{
	GENERATED_BODY()

public:
	UZWGameplayActionFactory();
	
	virtual FText GetDisplayName() const override;
	virtual FString GetDefaultNewAssetName() const override;
	
	virtual UObject* FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn) override;
};

#endif
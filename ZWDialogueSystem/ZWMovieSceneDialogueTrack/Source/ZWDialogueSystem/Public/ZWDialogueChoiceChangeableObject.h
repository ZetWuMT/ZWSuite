#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueChoiceChangeableObject.generated.h"

//DECLARE_MULTICAST_DELEGATE_OneParam(FOnValueChangedDelegate, UZWDialogueChoiceChangeableObject*);

UCLASS()
class ZWDIALOGUESYSTEM_API UZWDialogueChoiceChangeableObject : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueChangedDelegate, UZWDialogueChoiceChangeableObject*, NewValue);

	//UPROPERTY(BlueprintAssignable)
	FOnValueChangedDelegate OnChange;

	UFUNCTION(BlueprintCallable)
	void SetDirty(bool bTriggerOnChange = false);
	void ClearDirty();

	void BroadcastValueChanged();

private:
	bool bIsDirty = false;
};
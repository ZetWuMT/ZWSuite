#pragma once

#include "CoreMinimal.h"
#include "QuestChoiceChangeableObject.generated.h"

//DECLARE_MULTICAST_DELEGATE_OneParam(FOnValueChangedDelegate, UQuestChoiceChangeableObject*);

UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UQuestChoiceChangeableObject : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnValueChangedDelegate, UQuestChoiceChangeableObject*, NewValue);

	//UPROPERTY(BlueprintAssignable)
	FOnValueChangedDelegate OnChange;

	UFUNCTION(BlueprintCallable)
	void SetDirty(bool bTriggerOnChange = false);
	void ClearDirty();

	void BroadcastValueChanged();

private:
	bool bIsDirty = false;
};
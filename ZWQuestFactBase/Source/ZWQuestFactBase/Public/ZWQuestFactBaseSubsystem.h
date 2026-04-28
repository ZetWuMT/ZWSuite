// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWQuestFact.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ZWQuestFactBaseSubsystem.generated.h"

/**
 * 
 */

const int32 DefaultFactValue = 0;

USTRUCT(BlueprintType)
struct FZWQuestFactData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	FName Name;

	UPROPERTY(BlueprintReadWrite)
	FGuid Guid;

	TSharedPtr<UZWQuestFact> Parent;

	TArray<TSharedPtr<UZWQuestFact>> Children;

	UPROPERTY(BlueprintReadWrite)
	int32 Value = DefaultFactValue;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnFactValueChanged, FName, inName, int32, inValue);

UCLASS()
class ZWQUESTFACTBASE_API UZWQuestFactBaseSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:

	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	FOnFactValueChanged OnFactValueChanged;

	TArray<FZWQuestFactData*> QuestFacts;

	void RegisterFacts();

	UFUNCTION(BlueprintCallable, Category = "QuestFactBaseSubsystem")
	TArray<FZWQuestFactData>& GetFacts();

	UFUNCTION(BlueprintCallable,  Category = "QuestFactBaseSubsystem")
	void SetFactValue(FName inName, int32 inValue);

	UFUNCTION(BlueprintCallable, Category = "QuestFactBaseSubsystem")
	int32 GetFactValue(FName inName);

	TArray<FZWQuestFactData> FactsToPrint;
};

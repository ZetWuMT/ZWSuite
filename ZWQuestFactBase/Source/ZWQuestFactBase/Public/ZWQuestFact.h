// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWQuestFact.generated.h"

USTRUCT(BlueprintType)
struct FZWQuestFactSearchableName
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName QuestFactName;
};

UENUM(BlueprintType)
enum class QuestFactCompareType : uint8 
{
	Equal,
	Less,
	Greater,
	LessEqual,
	GreaterEqual,
	Not
};

USTRUCT(BlueprintType)
struct ZWQUESTFACTBASE_API FZWQuestFactCondition
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition")
    bool bUseCondition = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (EditCondition = "bUseCondition", GetOptions = "GetFactNames"))
    FZWQuestFactSearchableName FactName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (EditCondition = "bUseCondition"))
    QuestFactCompareType CompareType = QuestFactCompareType::Equal;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Condition", meta = (EditCondition = "bUseCondition"))
    int32 FactValue = 1;

    bool Evaluate(const UObject* WorldContextObject) const;
};

/**
 * 
 */
UCLASS(BlueprintType)
class ZWQUESTFACTBASE_API UZWQuestFact : public UObject
{
	GENERATED_BODY()

public:

	const FPrimaryAssetType PrimaryAssetType;

	FPrimaryAssetId GetPrimaryAssetId() const;

	UZWQuestFact();

	bool IsFolder();

protected:
	bool bIsFolder = false;

public:

	UPROPERTY(EditAnywhere)
	FGuid FactGuid;

	UPROPERTY(EditAnywhere)
	FName FactName;

	//UPROPERTY()
	UPROPERTY(EditAnywhere)
	FGuid ParentId;

	FGuid GetGuid();

	TArray<UZWQuestFact*> SubFacts;
};

/**
 *
 */
UCLASS(BlueprintType)
class ZWQUESTFACTBASE_API UZWQuestFolder : public UZWQuestFact
{
	GENERATED_BODY()

	UZWQuestFolder();
};

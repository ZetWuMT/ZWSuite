// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScatteringBase/ZWScatterer.h"
#include "ZWPawnScatterer.generated.h"

USTRUCT(BLueprintType)
struct FZWPawnScatterEntry : public FZWScatterEntry
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Enemy")
	TSubclassOf<AActor> EnemyClass;
};

USTRUCT()
struct FZWPawnSpawnParams : public FZWEntrySpawnParams
{
	GENERATED_BODY()
};

UCLASS()
class ZWSCATTERINGTOOL_API AZWPawnScatterer : public AZWScatterer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZWPawnScatterer();

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Scatter Configuration")
	TArray<FZWPawnScatterEntry> ScatterEntryTable;
	
private:
	virtual void PerformScattering(const TArray<AZWScatterProbe*>& AvailableProbes) override;
};

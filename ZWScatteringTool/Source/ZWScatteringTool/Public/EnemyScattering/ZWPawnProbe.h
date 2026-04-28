// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ScatteringBase/ZWScatterProbe.h"
#include "ZWPawnProbe.generated.h"

UCLASS()
class ZWSCATTERINGTOOL_API AZWPawnProbe : public AZWScatterProbe
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZWPawnProbe();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};

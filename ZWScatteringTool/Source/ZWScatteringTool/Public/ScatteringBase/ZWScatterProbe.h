// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/Actor.h"
#include "ZWScatterProbe.generated.h"

UCLASS(Abstract)
class ZWSCATTERINGTOOL_API AZWScatterProbe : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AZWScatterProbe();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Loot Probe")
	FGameplayTagContainer LocationTags;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot Probe")
	class UBillboardComponent* EditorBillboard;
#endif
};

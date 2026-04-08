// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "ZWUIPanelDatabase.generated.h"

class UZWUIPanel;

UENUM(BlueprintType)
enum class EZWWidgetLayer : uint8
{
	GameLayer,
	MenuLayer,
	PromptLayer
};

USTRUCT(BlueprintType)
struct FZWUIPanelData
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag PanelTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UZWUIPanel> PanelClass;
};

/**
 * 
 */
UCLASS()
class ZWUICORE_API UZWUIPanelDatabase : public UDataAsset
{
	GENERATED_BODY()
	
public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "ZW UI")
	TArray<FZWUIPanelData> Panels;
};

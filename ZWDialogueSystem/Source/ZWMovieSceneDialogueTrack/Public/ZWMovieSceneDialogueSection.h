// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "ZWMovieSceneDialogueSection.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UZWMovieSceneDialogueSection : public UMovieSceneSection
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Speaker")
	FText Speaker = FText::FromString("");

	UPROPERTY(EditAnywhere, Category = "Dialogue")
	FText DialogueText = FText::FromString("");
	
};

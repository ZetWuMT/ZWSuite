// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MovieSceneSection.h"
#include "ZWDialogueData.h"
#include "ZWMovieSceneDialogueSection.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UZWMovieSceneDialogueSection : public UMovieSceneSection
{
	GENERATED_BODY()

public:
	//UPROPERTY(VisibleAnywhere, Category = "Guid")
	//FGuid EventID;
	
	//UPROPERTY(EditAnywhere, Category = "Speaker")
	//FName Speaker = FName("");
	
	//UPROPERTY(EditAnywhere, Category = "Speaker")
	//FText LocalizedSpeaker = FText::FromString("");

	//UPROPERTY(EditAnywhere, Category = "Dialogue")
	//FText DialogueText = FText::FromString("");
	
	UPROPERTY(EditAnywhere)
	FZWDialogueData DialogueData;
	
};

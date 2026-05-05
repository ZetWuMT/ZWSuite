// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EntitySystem/TrackInstance/MovieSceneTrackInstance.h"
#include "ZWMovieSceneDialogueTrackInstance.generated.h"

/**
 * 
 */
UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UZWMovieSceneDialogueTrackInstance : public UMovieSceneTrackInstance
{
	GENERATED_BODY()

private:
	virtual void OnInputAdded(const FMovieSceneTrackInstanceInput& InInput) override;
	virtual void OnInputRemoved(const FMovieSceneTrackInstanceInput& InInput) override;
	virtual void OnDestroyed() override;

private:
	class UZWMovieSceneDialogueWidget* MovieSceneDialogueWidget = nullptr;
};

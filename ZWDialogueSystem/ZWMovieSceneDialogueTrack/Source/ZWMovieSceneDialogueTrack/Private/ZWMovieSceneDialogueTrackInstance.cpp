// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWMovieSceneDialogueTrackInstance.h"
#include "AssetRegistry/AssetRegistryModule.h"


void UZWMovieSceneDialogueTrackInstance::OnInputAdded(const FMovieSceneTrackInstanceInput& InInput)
{
	//using namespace UE::MovieScene;
	/*
	UMovieSceneEntitySystemLinker* Linker = GetLinker();
	UMovieSceneSection* Section = CastChecked<UMovieSceneSection>(InInput.Section);

	if (MovieSceneDialogueWidget == nullptr)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

		UE_LOG(LogTemp, Verbose, TEXT("OnInputAdded worked!"))
	}*/
}

void UZWMovieSceneDialogueTrackInstance::OnInputRemoved(const FMovieSceneTrackInstanceInput& InInput)
{
}

void UZWMovieSceneDialogueTrackInstance::OnDestroyed()
{
}
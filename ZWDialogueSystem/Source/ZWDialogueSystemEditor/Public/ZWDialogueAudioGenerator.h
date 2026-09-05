// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueData.h"
#include <atomic>
/**
 *
 */

DECLARE_DELEGATE_TwoParams(FOnTTSRequestCompleted, const FZWDialogueData& /*UpdatedData*/, bool /*bSuccess*/);

class ZWDIALOGUESYSTEMEDITOR_API FZWDialogueAudioGenerator : public TSharedFromThis<FZWDialogueAudioGenerator>
{
public:
	static TSharedRef<FZWDialogueAudioGenerator> Create()
	{
		return MakeShared<FZWDialogueAudioGenerator>();
	}

	void Execute(const FZWDialogueData& InData, const FString& PythonExePath, const FString& LangCode, FOnTTSRequestCompleted InCallback);

private:
	FZWDialogueData WorkingData;
	FString TargetLang;
	FOnTTSRequestCompleted CompletionCallback;
	std::atomic<bool> bInFlight{false};
};

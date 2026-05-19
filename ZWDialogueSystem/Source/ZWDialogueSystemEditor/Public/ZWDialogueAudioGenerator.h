// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWDialogueData.h"
#include "Interfaces/IHttpRequest.h"
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
	
	void Execute(const FZWDialogueData& InData, const FString& ApiKey, const FString& LangCode, FOnTTSRequestCompleted InCallback);

private:
	void OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful);

	FZWDialogueData WorkingData;
	FString TargetLang;
	FOnTTSRequestCompleted CompletionCallback;
};

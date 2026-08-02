// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWDialogueAudioGenerator.h"
#include "ZWDialogueData.h"
#include "HttpModule.h"
#include "ZWDialogueSettings.h"
#include "Interfaces/IHttpResponse.h"

void FZWDialogueAudioGenerator::Execute(const FZWDialogueData& InData, const FString& ApiKey, const FString& LangCode, FOnTTSRequestCompleted InCallback)
{
	// 1. Guards and GUID generation if the line does not have one
	WorkingData = InData;
	TargetLang = LangCode;
	CompletionCallback = InCallback;

	if (WorkingData.DialogueLine.IsEmpty())
	{
		CompletionCallback.ExecuteIfBound(WorkingData, false);
		return;
	}

	if (!WorkingData.AudioData.AudioGuid.IsValid())
	{
		WorkingData.AudioData.AudioGuid = FGuid::NewGuid();
	}

    // 2. Building the JSON structure for Google TTS
    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject());

    TSharedPtr<FJsonObject> InputObj = MakeShareable(new FJsonObject());
    InputObj->SetStringField(TEXT("text"), InData.DialogueLine.ToString());
    RequestObj->SetObjectField(TEXT("input"), InputObj);

    TSharedPtr<FJsonObject> VoiceObj = MakeShareable(new FJsonObject());
    VoiceObj->SetStringField(TEXT("languageCode"), LangCode); 
    // In a professional tool you would pick the voice "name" dynamically based on SpeakerId
    //VoiceObj->SetStringField(TEXT("name"), LangCode == "pl-PL" ? "pl-PL-Wavenet-B" : "en-GB-Chirp3-HD-Aoede");
	FString SpeakerName = "";
	if (const UZWDialogueSettings* DialogueSettings = GetDefault<UZWDialogueSettings>())
	{
		if (DialogueSettings->AudioGenerationData)
		{
			SpeakerName = *DialogueSettings->AudioGenerationData->SpeakerVoiceNames.Find(InData.SpeakerID);	
			if (SpeakerName == "")
			{
				SpeakerName = DialogueSettings->DefaultVoiceName;
			}
		}		
	}
	VoiceObj->SetStringField(TEXT("name"), SpeakerName);
    RequestObj->SetObjectField(TEXT("voice"), VoiceObj);

    TSharedPtr<FJsonObject> AudioConfigObj = MakeShareable(new FJsonObject());
    AudioConfigObj->SetStringField(TEXT("audioEncoding"), "LINEAR16"); // Returns a clean WAV file
    AudioConfigObj->SetNumberField(TEXT("sampleRateHertz"), 48000);
    RequestObj->SetObjectField(TEXT("audioConfig"), AudioConfigObj);

    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    // 3. HTTP request configuration
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString Endpoint = FString::Printf(TEXT("https://texttospeech.googleapis.com/v1/text:synthesize?key=%s"), *ApiKey);
    
    Request->SetURL(Endpoint);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonPayload);

    // Prepare the save path (e.g. Content/Localization/Audio/db-db/1234-5678.wav)
    FString FileName = InData.AudioData.AudioGuid.ToString() + TEXT(".wav");
    FString SavePath = FPaths::ProjectContentDir() / TEXT("Localization/Audio") / LangCode / FileName;

    // 4. Bind the response and send it
	TSharedRef<FZWDialogueAudioGenerator> StrongThis = AsShared();
    
	// Bind via a lambda that keeps StrongThis in memory until the request finishes
	Request->OnProcessRequestComplete().BindLambda([StrongThis](FHttpRequestPtr Req, FHttpResponsePtr Res, bool bWasSuccessful)
	{
		StrongThis->OnTTSResponseReceived(Req, Res, bWasSuccessful);
	});

	Request->ProcessRequest();
}

void FZWDialogueAudioGenerator::OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
	UE_LOG(LogTemp, Warning, TEXT("OnTTSResponseReceived called."));
	
	bool bSuccess = false;
	
	if (bWasSuccessful && Response.IsValid() && Response->GetResponseCode() == 200)
	{
		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Response->GetContentAsString());

		if (FJsonSerializer::Deserialize(Reader, JsonObject) && JsonObject->HasField(TEXT("audioContent")))
		{
			FString Base64Audio = JsonObject->GetStringField(TEXT("audioContent"));
			TArray<uint8> AudioBytes;
			FBase64::Decode(Base64Audio, AudioBytes);

			if (AudioBytes.Num() > 44)
			{
				uint32 ByteRate = 0;
				FMemory::Memcpy(&ByteRate, AudioBytes.GetData() + 28, 4);

				if (ByteRate > 0)
				{
					WorkingData.AudioData.PrecalculatedDuration = (float)(AudioBytes.Num() - 44) / (float)ByteRate;
				}
				
				UE_LOG(LogTemp, Warning, TEXT("Calculated duration: %f seconds"), WorkingData.AudioData.PrecalculatedDuration);

				FString FileName = WorkingData.AudioData.AudioGuid.ToString() + TEXT(".wav");
				FString SavePath = FPaths::ProjectContentDir() / TEXT("Localization/Audio") / TargetLang / FileName;
                
				if (FFileHelper::SaveArrayToFile(AudioBytes, *SavePath))
				{
					bSuccess = true;
				}
			}
		}
	}
	else
	{
		FString ErrorMsg = Response.IsValid() ? Response->GetContentAsString() : TEXT("No response");
		UE_LOG(LogTemp, Error, TEXT("TTS error: %s"), *ErrorMsg);
	}

	// Call the callback with the updated structure
	CompletionCallback.ExecuteIfBound(WorkingData, bSuccess);
}
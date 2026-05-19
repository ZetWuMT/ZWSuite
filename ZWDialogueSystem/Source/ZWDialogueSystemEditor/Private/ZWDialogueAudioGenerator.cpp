// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWDialogueAudioGenerator.h"
#include "ZWDialogueData.h"
#include "HttpModule.h"
#include "Interfaces/IHttpResponse.h"

void FZWDialogueAudioGenerator::Execute(const FZWDialogueData& InData, const FString& ApiKey, const FString& LangCode, FOnTTSRequestCompleted InCallback)
{
	// 1. Zabezpieczenia i generowanie GUID, jeśli linia go nie posiada
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

    // 2. Budowanie struktury JSON dla Google TTS
    TSharedPtr<FJsonObject> RequestObj = MakeShareable(new FJsonObject());

    TSharedPtr<FJsonObject> InputObj = MakeShareable(new FJsonObject());
    InputObj->SetStringField(TEXT("text"), InData.DialogueLine.ToString());
    RequestObj->SetObjectField(TEXT("input"), InputObj);

    TSharedPtr<FJsonObject> VoiceObj = MakeShareable(new FJsonObject());
    VoiceObj->SetStringField(TEXT("languageCode"), LangCode); 
    // W profesjonalnym narzędziu "name" głosu dobierałbyś dynamicznie na podstawie SpeakerId
    VoiceObj->SetStringField(TEXT("name"), LangCode == "pl-PL" ? "pl-PL-Wavenet-B" : "en-GB-Chirp3-HD-Aoede"); 
    RequestObj->SetObjectField(TEXT("voice"), VoiceObj);

    TSharedPtr<FJsonObject> AudioConfigObj = MakeShareable(new FJsonObject());
    AudioConfigObj->SetStringField(TEXT("audioEncoding"), "LINEAR16"); // Zwróci czysty plik WAV
    AudioConfigObj->SetNumberField(TEXT("sampleRateHertz"), 48000);
    RequestObj->SetObjectField(TEXT("audioConfig"), AudioConfigObj);

    FString JsonPayload;
    TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&JsonPayload);
    FJsonSerializer::Serialize(RequestObj.ToSharedRef(), Writer);

    // 3. Konfiguracja żądania HTTP
    TSharedRef<IHttpRequest, ESPMode::ThreadSafe> Request = FHttpModule::Get().CreateRequest();
    FString Endpoint = FString::Printf(TEXT("https://texttospeech.googleapis.com/v1/text:synthesize?key=%s"), *ApiKey);
    
    Request->SetURL(Endpoint);
    Request->SetVerb("POST");
    Request->SetHeader("Content-Type", "application/json");
    Request->SetContentAsString(JsonPayload);

    // Przygotowanie ścieżki zapisu (np. Content/Localization/Audio/db-db/1234-5678.wav)
    FString FileName = InData.AudioData.AudioGuid.ToString() + TEXT(".wav");
    FString SavePath = FPaths::ProjectContentDir() / TEXT("Localization/Audio") / LangCode / FileName;

    // 4. Bindowanie odpowiedzi i wysyłka
	Request->OnProcessRequestComplete().BindSP(this, &FZWDialogueAudioGenerator::OnTTSResponseReceived);
	Request->ProcessRequest();
}

void FZWDialogueAudioGenerator::OnTTSResponseReceived(FHttpRequestPtr Request, FHttpResponsePtr Response, bool bWasSuccessful)
{
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
		FString ErrorMsg = Response.IsValid() ? Response->GetContentAsString() : TEXT("Brak odpowiedzi");
		UE_LOG(LogTemp, Error, TEXT("Błąd TTS: %s"), *ErrorMsg);
	}

	// Wywołanie callbacka ze zaktualizowaną strukturą
	CompletionCallback.ExecuteIfBound(WorkingData, bSuccess);
}
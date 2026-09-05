// Fill out your copyright notice in the Description page of Project Settings.

#include "ZWDialogueAudioGenerator.h"
#include "ZWDialogueData.h"
#include "ZWDialogueSettings.h"
#include "Misc/FileHelper.h"
#include "HAL/PlatformFileManager.h"
#include "Async/Async.h"

THIRD_PARTY_INCLUDES_START
#include "ThirdParty/dr_mp3.h"
#include "ThirdParty/dr_wav.h"
THIRD_PARTY_INCLUDES_END

// Decodes the edge-tts MP3 output and writes a standard PCM16 WAV, returning its duration in seconds.
static bool ConvertMp3ToWav(const FString& InMp3Path, const FString& OutWavPath, float& OutDurationSeconds)
{
	OutDurationSeconds = 0.0f;

	TArray64<uint8> Mp3Bytes;
	if (!FFileHelper::LoadFileToArray(Mp3Bytes, *InMp3Path) || Mp3Bytes.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] MP3 output is missing or empty: %s"), *InMp3Path);
		return false;
	}

	drmp3_config Mp3Config = {};
	drmp3_uint64 TotalFrames = 0;
	float* Frames = drmp3_open_memory_and_read_pcm_frames_f32(Mp3Bytes.GetData(), Mp3Bytes.Num(), &Mp3Config, &TotalFrames, nullptr);
	if (!Frames || TotalFrames == 0 || Mp3Config.channels == 0 || Mp3Config.sampleRate == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Failed to decode MP3: %s"), *InMp3Path);
		drmp3_free(Frames, nullptr);
		return false;
	}

	const drmp3_uint64 TotalSamples = TotalFrames * Mp3Config.channels;
	TArray<uint8> WavData;
	WavData.SetNumUninitialized(static_cast<int32>(TotalSamples * sizeof(int16)));
	int16* S16 = reinterpret_cast<int16*>(WavData.GetData());
	for (drmp3_uint64 Sample = 0; Sample < TotalSamples; ++Sample)
	{
		S16[Sample] = static_cast<int16>(FMath::Clamp(FMath::RoundToInt(Frames[Sample] * 32767.0f), -32768, 32767));
	}
	drmp3_free(Frames, nullptr);

	drwav_data_format Format = {};
	Format.container = drwav_container_riff;
	Format.format = DR_WAVE_FORMAT_PCM;
	Format.channels = static_cast<drmp3_uint32>(Mp3Config.channels);
	Format.sampleRate = static_cast<drmp3_uint32>(Mp3Config.sampleRate);
	Format.bitsPerSample = 16;

	void* WavBuffer = nullptr;
	size_t WavSize = 0;
	drwav Wav;
	if (!drwav_init_memory_write_sequential(&Wav, &WavBuffer, &WavSize, &Format, TotalSamples, nullptr))
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Failed to initialize WAV writer for: %s"), *OutWavPath);
		return false;
	}
	drwav_write_pcm_frames(&Wav, TotalFrames, S16);
	drwav_uninit(&Wav);

	const bool bSaved = FFileHelper::SaveArrayToFile(TArray<uint8>(static_cast<uint8*>(WavBuffer), static_cast<int32>(WavSize)), *OutWavPath);
	drwav_free(WavBuffer, nullptr);

	if (bSaved)
	{
		OutDurationSeconds = static_cast<float>(TotalFrames) / static_cast<float>(Mp3Config.sampleRate);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Failed to save WAV file: %s"), *OutWavPath);
	}
	return bSaved;
}

void FZWDialogueAudioGenerator::Execute(const FZWDialogueData& InData, const FString& PythonExePath, const FString& LangCode, FOnTTSRequestCompleted InCallback)
{
	if (bInFlight.exchange(true))
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Execute() re-entered while a generation is already in flight on this instance; refusing."));
		InCallback.ExecuteIfBound(InData, false);
		return;
	}

	// 1. Guards and GUID generation if the line does not have one
	WorkingData = InData;
	TargetLang = LangCode;
	CompletionCallback = InCallback;

	if (WorkingData.DialogueLine.IsEmpty())
	{
		bInFlight = false;
		CompletionCallback.ExecuteIfBound(WorkingData, false);
		return;
	}

	if (!WorkingData.AudioData.AudioGuid.IsValid())
	{
		WorkingData.AudioData.AudioGuid = FGuid::NewGuid();
	}

	// 2. Resolve the Edge voice name for this speaker
	FString VoiceName;
	if (const UZWDialogueSettings* DialogueSettings = GetDefault<UZWDialogueSettings>())
	{
		if (DialogueSettings->AudioGenerationData)
		{
			VoiceName = DialogueSettings->AudioGenerationData->SpeakerVoiceNames.FindRef(InData.SpeakerID);
		}
		if (VoiceName.IsEmpty())
		{
			VoiceName = DialogueSettings->DefaultVoiceName;
		}
	}

	if (VoiceName.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[ZW TTS] No voice resolved for speaker '%s' and DefaultVoiceName is empty."), *InData.SpeakerID.ToString());
		bInFlight = false;
		CompletionCallback.ExecuteIfBound(WorkingData, false);
		return;
	}

	// 3. Run edge-tts on a worker thread, convert, save, then fire the callback on the game thread
	TSharedRef<FZWDialogueAudioGenerator> StrongThis = AsShared();

	Async(EAsyncExecution::ThreadPool, [StrongThis, PythonExePath, VoiceName]()
	{
		bool bSuccess = false;

		const FString TempDir = FPlatformProcess::UserTempDir();
		const FString TempTextPath = TempDir / (StrongThis->WorkingData.AudioData.AudioGuid.ToString() + TEXT(".txt"));
		const FString TempMediaPath = TempDir / (StrongThis->WorkingData.AudioData.AudioGuid.ToString() + TEXT(".mp3"));

		if (!FFileHelper::SaveStringToFile(StrongThis->WorkingData.DialogueLine.ToString(), *TempTextPath, FFileHelper::EEncodingOptions::ForceUTF8WithoutBOM))
		{
			UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Failed to write temp text file: %s"), *TempTextPath);
		}
		else
		{
			const FString RunnerScriptPath = FPaths::ProjectPluginsDir() / TEXT("ZW/ZWDialogueSystem/Source/ZWDialogueSystemEditor/Private/Scripts/ZWEdgeTTSRunner.py");
			const FString Params = FString::Printf(TEXT("\"%s\" \"%s\" \"%s\" \"%s\""), *RunnerScriptPath, *TempTextPath, *VoiceName, *TempMediaPath);

			int32 ReturnCode = -1;
			FString StdOut;
			FString StdErr;
			FPlatformProcess::ExecProcess(*PythonExePath, *Params, &ReturnCode, &StdOut, &StdErr);

			if (ReturnCode != 0)
			{
				UE_LOG(LogTemp, Error, TEXT("[ZW TTS] edge-tts failed (exit %d). If edge-tts is missing run: pip install edge-tts. StdErr: %s"), ReturnCode, *StdErr);
			}
			else
			{
				const FString SaveDir = FPaths::ProjectContentDir() / TEXT("Localization/Audio") / StrongThis->TargetLang;
				IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();
				if (!PlatformFile.CreateDirectoryTree(*SaveDir))
				{
					UE_LOG(LogTemp, Error, TEXT("[ZW TTS] Failed to create directory: %s"), *SaveDir);
				}
				else
				{
					const FString SavePath = SaveDir / (StrongThis->WorkingData.AudioData.AudioGuid.ToString() + TEXT(".wav"));
					float Duration = 0.0f;
					if (ConvertMp3ToWav(TempMediaPath, SavePath, Duration))
					{
						StrongThis->WorkingData.AudioData.PrecalculatedDuration = Duration;
						UE_LOG(LogTemp, Log, TEXT("[ZW TTS] Generated %s (%.2fs, voice %s)"), *SavePath, Duration, *VoiceName);
						bSuccess = true;
					}
				}
			}
		}

		IPlatformFile& CleanupFile = FPlatformFileManager::Get().GetPlatformFile();
		CleanupFile.DeleteFile(*TempTextPath);
		CleanupFile.DeleteFile(*TempMediaPath);

		AsyncTask(ENamedThreads::GameThread, [StrongThis, bSuccess]()
		{
			StrongThis->CompletionCallback.ExecuteIfBound(StrongThis->WorkingData, bSuccess);
			StrongThis->bInFlight = false;
		});
	});
}

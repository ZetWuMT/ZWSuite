// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWQuestFactBaseSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"

void UZWQuestFactBaseSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	RegisterFacts();
}

void UZWQuestFactBaseSubsystem::Deinitialize()
{
}

void UZWQuestFactBaseSubsystem::RegisterFacts()
{
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	TArray<FAssetData> UQuestFactAssetDataArray;
	AssetRegistry.GetAssetsByClass(UZWQuestFact::StaticClass()->GetClassPathName(), UQuestFactAssetDataArray, true);

	for (FAssetData FAssetData : UQuestFactAssetDataArray)
	{
		FString Path = FAssetData.GetObjectPathString();
		UZWQuestFact* Fact = LoadObject<UZWQuestFact>(GetTransientPackage(), *Path);

		FZWQuestFactData* QuestFactData = new FZWQuestFactData();
		QuestFactData->Guid = Fact->FactGuid;
		QuestFactData->Name = Fact->FactName;

		QuestFacts.Add(QuestFactData);
	}
}

TArray<FZWQuestFactData>& UZWQuestFactBaseSubsystem::GetFacts()
{
	for (auto Fact : QuestFacts)
	{
		FactsToPrint.Add(*Fact);
	}
	return FactsToPrint;
}

void UZWQuestFactBaseSubsystem::SetFactValue(FName inName, int32 inValue)
{
	for (auto Fact : QuestFacts)
	{
		if (Fact->Name == inName)
		{
			Fact->Value = inValue;
			OnFactValueChanged.Broadcast(inName, inValue);
		}
	}
}



int32 UZWQuestFactBaseSubsystem::GetFactValue(FName inName)
{
	int32 Value = 0;
	for (auto Fact : QuestFacts)
	{
		if (Fact->Name == inName)
		{
			Value =  Fact->Value;
		}
	}

	return Value;
}

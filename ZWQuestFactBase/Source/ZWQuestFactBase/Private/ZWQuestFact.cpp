// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWQuestFact.h"
#include "ZWQuestFactBaseSubsystem.h"
#include "Kismet/GameplayStatics.h"

bool FZWQuestFactCondition::Evaluate(const UObject* WorldContextObject) const
{
	if (!bUseCondition || !WorldContextObject)
	{
		return true;
	}

	if (const UGameInstance* GameInstance = UGameplayStatics::GetGameInstance(WorldContextObject))
	{
		if (UZWQuestFactBaseSubsystem* QuestFactBaseSubsystem = GameInstance->GetSubsystem<UZWQuestFactBaseSubsystem>())
		{
			const int32 CurrentFactValue = QuestFactBaseSubsystem->GetFactValue(FactName.QuestFactName);

			switch(CompareType)
			{
				case QuestFactCompareType::Less:
					return FactValue < CurrentFactValue;
				case QuestFactCompareType::Greater:
					return FactValue > CurrentFactValue;
				case QuestFactCompareType::LessEqual:
					return FactValue <= CurrentFactValue;
				case QuestFactCompareType::GreaterEqual:
					return FactValue >= CurrentFactValue;
				case QuestFactCompareType::Not:
					return FactValue != CurrentFactValue;
				case QuestFactCompareType::Equal:
				default:
					return FactValue == CurrentFactValue;
			}
		}
	}

	return false;
}

FPrimaryAssetId UZWQuestFact::GetPrimaryAssetId() const
{
	return FPrimaryAssetId(PrimaryAssetType, FName(GetName()));
}

UZWQuestFact::UZWQuestFact()
{
	
}

FGuid UZWQuestFact::GetGuid()
{
	return FactGuid;
}

bool UZWQuestFact::IsFolder()
{
	return bIsFolder;
}

UZWQuestFolder::UZWQuestFolder()
{
	bIsFolder = true;
}

// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWQuestFact.h"

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

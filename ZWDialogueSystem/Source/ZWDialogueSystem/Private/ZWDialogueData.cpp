#include "ZWDialogueData.h"

FZWDialogueData::FZWDialogueData()
	: EventID(FGuid::NewGuid())
{
}

bool FZWDialogueData::operator==(const FZWDialogueData& Other) const
{	
	if (EventID != Other.EventID) return false;
	if (SpeakerID != Other.SpeakerID) return false;
	if (!Speaker.EqualTo(Other.Speaker)) return false;
	if (!DialogueLine.EqualTo(Other.DialogueLine)) return false;
	if (AudioData.AudioGuid != Other.AudioData.AudioGuid) return false;
	
	return true;
}

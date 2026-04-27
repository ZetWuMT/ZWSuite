#include "QuestChoiceChangeableObject.h"

void UQuestChoiceChangeableObject::SetDirty(bool bTriggerOnChange)
{
    bIsDirty = true;
    if (bTriggerOnChange)
    {
        BroadcastValueChanged();
    }
}

void UQuestChoiceChangeableObject::ClearDirty()
{
    bIsDirty = false;
}

void UQuestChoiceChangeableObject::BroadcastValueChanged()
{
    if (bIsDirty)
    {
        OnChange.Broadcast(this);
    }
    ClearDirty();
}
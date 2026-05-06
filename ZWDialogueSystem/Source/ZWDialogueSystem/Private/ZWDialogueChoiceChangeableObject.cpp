#include "ZWDialogueChoiceChangeableObject.h"

void UZWDialogueChoiceChangeableObject::SetDirty(bool bTriggerOnChange)
{
    bIsDirty = true;
    if (bTriggerOnChange)
    {
        BroadcastValueChanged();
    }
}

void UZWDialogueChoiceChangeableObject::ClearDirty()
{
    bIsDirty = false;
}

void UZWDialogueChoiceChangeableObject::BroadcastValueChanged()
{
    if (bIsDirty)
    {
        OnChange.Broadcast(this);
    }
    ClearDirty();
}
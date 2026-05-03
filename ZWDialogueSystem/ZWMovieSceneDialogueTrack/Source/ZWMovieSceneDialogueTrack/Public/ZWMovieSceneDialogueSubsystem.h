#pragma once

#include "Subsystems/GameInstanceSubsystem.h"
#include "UObject/Interface.h"
#include "UObject/WeakInterfacePtr.h"
#include "ZWMovieSceneDialogueSubsystem.generated.h"

struct FZWDialogueData;
class ULevelSequence;
class FZWDialogueToken;
class IZWDialogueLineHandler;

using FZWDialogueTokenPtr = TSharedPtr<FZWDialogueToken>;


//TODO: Potential multi-casts for additional subsystems if needed
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueStartedEvent, const FGuid);
DECLARE_MULTICAST_DELEGATE_OneParam(FDialogueEndedEvent, const FGuid);
DECLARE_MULTICAST_DELEGATE_TwoParams(FDialogueTickEvent, const FGuid, float);


USTRUCT()
struct FZWDialogueData
{
    GENERATED_BODY()

public:
    FGuid EventID;

    UPROPERTY(EditAnywhere)
    FText Speaker;

    UPROPERTY(EditAnywhere)
    FText DialogueLine;

    TWeakInterfacePtr<IZWDialogueLineHandler> FinalDialogueLineHandler = nullptr;
};

class FZWDialogueToken
{
public:
    FZWDialogueToken(UZWMovieSceneDialogueSubsystem* DialogueSubsystem, const FGuid& DialogueEventID);
    ~FZWDialogueToken();

public:
    FGuid EventID;

private:
    TWeakObjectPtr<UZWMovieSceneDialogueSubsystem> DialogueSubsystemWeak;
};

struct FZWDialogueDetails
{
    FText Speaker;
    FText DialogueText;

    float Progress; // In Seconds
};

enum class EZWStartDialogueResult
{
    Handled,
    Unhandled,
    Final
};

UINTERFACE(Blueprintable)
class ZWMOVIESCENEDIALOGUETRACK_API UZWDialogueLineHandler : public UInterface
{
    GENERATED_BODY()
};

class ZWMOVIESCENEDIALOGUETRACK_API IZWDialogueLineHandler
{
    GENERATED_BODY()

public:
    virtual uint32 GetOrder() const = 0;

    virtual EZWStartDialogueResult OnStartDialogueLine(const FZWDialogueData& DialogueData) = 0;
    virtual void OnFinishDialogueLine(const FZWDialogueData& DialogueData) = 0;
    virtual void OnDialogueLineUpdated(const FZWDialogueData& DialogueData) {}
};

UCLASS()
class ZWMOVIESCENEDIALOGUETRACK_API UZWMovieSceneDialogueSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    
    //@TODO: Potential multi-casts for additional subsystems if needed
    FDialogueStartedEvent DialogueStartedEvent;
    FDialogueEndedEvent DialogueEndedEvent;
    FDialogueTickEvent DialogueTickEvent;
    
    virtual void Deinitialize() override;

    void TickDialogue(FGuid EventID, float CurrentTime);

    FZWDialogueTokenPtr TriggerDialogueLine(const FZWDialogueDetails& DialogueLine);
    FZWDialogueTokenPtr TriggerDialogueLine(FZWDialogueData DialogueData);

    void RegisterDialogueHandler(IZWDialogueLineHandler* Handler);
    void UnregisterDialogueHandler(IZWDialogueLineHandler* Handler);

private: 
    void CloseDialogueLine(const FGuid& EventID);
    
    TArray<FZWDialogueData> CurrentDialogueLines;

    TArray<TWeakInterfacePtr<IZWDialogueLineHandler>> DialogueLineHandlers;

    friend FZWDialogueToken;
};
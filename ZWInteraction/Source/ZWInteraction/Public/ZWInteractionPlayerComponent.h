// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ZWInteractionComponent.h"
#include "Components/ArrowComponent.h"
#include "ZWInteractionPlayerComponent.generated.h"

class UZWInteractionComponent;

UCLASS( ClassGroup=(Custom), BlueprintType, meta=(BlueprintSpawnableComponent) )
class ZWINTERACTION_API UZWInteractionPlayerComponent : public UArrowComponent
{
	GENERATED_BODY()
	
public:	
	UZWInteractionPlayerComponent();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	UFUNCTION(BlueprintCallable)
	virtual void Interact();
	
	UFUNCTION(BlueprintImplementableEvent)
	void GetInteractableObjectName(const FString& InteractableObjectName);

protected:
	virtual void ResolveLineTracePoints(FVector& TraceStart, FVector& TraceEnd, float& TraceRadius);
	virtual void ResolveLineTraceIgnoredActors(TArray<AActor*>& ActorsToIgnore);
	
	virtual UZWInteractionComponent* GetInteractableObjectInteractionComponent(AActor* InteractableActor);
	
	TObjectPtr<UZWInteractionComponent> GetInteractableObject() { return InteractableObject; }
	void SetInteractableObject(TObjectPtr<UZWInteractionComponent> Object);
	void ResetInteractableObject();
	
	TObjectPtr<UZWInteractionComponent> GetInteractedObject() { return InteractedObject; }
	void SetInteractedObject(TObjectPtr<UZWInteractionComponent> Object);
	void ResetInteractedObject();
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin="0", UIMin="0", ClampMax="20", UIMax="20", AllowPrivateAccess="true"))
	float DetectionTraceRadius = 7.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(ClampMin="0", UIMin="0", ClampMax="2000", UIMax="2000", AllowPrivateAccess="true"))
	float DetectionRange = 400.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(AllowPrivateAccess="true"))
	bool bDrawDebugTrace = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Interaction|Debug", meta=(AllowPrivateAccess="true"))
	bool bPrintInteractableObjectName = false;

private:	
	void DetectInteractiveObjects();
	void ResolveInteractiveObject(AActor* NewInteractableObject);
	
	UPROPERTY()
	TObjectPtr<UZWInteractionComponent> InteractableObject = nullptr;

	UPROPERTY()
	TObjectPtr<UZWInteractionComponent> InteractedObject = nullptr;
};
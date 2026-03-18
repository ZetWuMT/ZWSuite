// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/SceneCapture2D.h"
#include "ZWInteractionSceneCapture.generated.h"

/**
 * 
 */
UCLASS()
class ZWINTERACTION_API AZWInteractionSceneCapture : public ASceneCapture2D
{
	GENERATED_BODY()

	AZWInteractionSceneCapture(const FObjectInitializer& ObjectInitializer);
	
	/** Item component. */
	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> ItemComponent;

	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class AActor> InspectedActor;

	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UPostProcessComponent> PostProcessComponent;

	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UStaticMeshComponent> CubeComponent;

	UPROPERTY(Category = DecalActor, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class URectLightComponent> RectLightComponent;

	void SetupPostProcessComponent();

	void SetupCubeComponent();

	void SetupRectLightComponent();

	void SetupInspectedActor(AActor* InputActor);

public:
	void UpdateVisibility(AActor* InActor);

	void AddInspectedActorLocation(FVector Location);
	void AddInspectedActorRotation(FRotator Rotation);
	void SetLookAtRotation(FVector Location);
};

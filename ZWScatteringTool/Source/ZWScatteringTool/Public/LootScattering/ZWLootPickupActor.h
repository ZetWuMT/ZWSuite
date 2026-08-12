// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "IPickupable.h"
#include "GameFramework/Actor.h"
#include "ZWLootPickupActor.generated.h"

class UStaticMesh;
class USceneComponent;
class UStaticMeshComponent;
class UZWInteractionComponent;
class UZWInventoryComponent;

UCLASS()
class ZWSCATTERINGTOOL_API AZWLootPickupActor : public AActor
{
	GENERATED_BODY()

public:
	AZWLootPickupActor();

	UFUNCTION(BlueprintCallable, Category = "Loot Pickup")
	void SetStaticMesh(UStaticMesh* InMesh);

	UFUNCTION(BlueprintCallable, Category = "Loot Pickup")
	void SetPickupInventory(const FInventoryPickup& InPickupInventory);

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USceneComponent> SceneRoot;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UStaticMeshComponent> StaticMeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZWInteractionComponent> InteractionComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Loot Pickup", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UZWInventoryComponent> InventoryComp;
};

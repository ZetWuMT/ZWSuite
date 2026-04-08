// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IPickupable.h"
#include "ZWInventoryComponent.generated.h"

class UZWInteractionComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ZWINVENTORY_API UZWInventoryComponent : public UActorComponent, public IPickupable
{
	GENERATED_BODY()

public:	

	UZWInventoryComponent();

	virtual FInventoryPickup GetPickupInventory() const override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
	virtual void OnComponentDestroyed(bool bDestroyingHierarchy) override;	

	UFUNCTION()
	void SetupInteraction();	
	
	UPROPERTY(EditAnywhere)
	FInventoryPickup StaticInventory;

private:
	UZWInteractionComponent* InteractionComponent;
	
};

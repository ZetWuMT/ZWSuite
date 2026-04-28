// Fill out your copyright notice in the Description page of Project Settings.


#include "ScatteringBase/ZWScatterProbe.h"

#include "Components/BillboardComponent.h"

// Sets default values
AZWScatterProbe::AZWScatterProbe()
{
	PrimaryActorTick.bCanEverTick = false;
	
	USceneComponent* SceneRoot = CreateDefaultSubobject<USceneComponent>(FName("SceneRoot"));
	RootComponent = SceneRoot;
	
#if WITH_EDITORONLY_DATA
	EditorBillboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("EditorBillboard"));
	EditorBillboard->SetupAttachment(SceneRoot);
	EditorBillboard->SetSprite(LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EditorResources/Ai_Spawnpoint.Ai_Spawnpoint")));
#endif
}



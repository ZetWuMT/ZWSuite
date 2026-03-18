// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionSceneCapture.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Components/PostProcessComponent.h"
#include "Components/RectLightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetMathLibrary.h"


AZWInteractionSceneCapture::AZWInteractionSceneCapture(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	SetActorRotation(FRotator(0,90,90));
	
	ItemComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("InspectedActor"));
	ItemComponent->SetupAttachment(GetCaptureComponent2D());
	ItemComponent->SetRelativeLocation(FVector(50,0,0));
	ItemComponent->SetRelativeRotation(FRotator(0,90,90));

	PostProcessComponent = CreateDefaultSubobject<UPostProcessComponent>(TEXT("PostProcessComponent"));
	PostProcessComponent->SetupAttachment(RootComponent);

	CubeComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeComponent"));
	CubeComponent->SetupAttachment(RootComponent);

	RectLightComponent = CreateDefaultSubobject<URectLightComponent>(TEXT("RectLightComponent"));
	RectLightComponent->SetupAttachment(RootComponent);

	
	UTextureRenderTarget2D* TextureRenderTarget2D = ConstructorHelpers::FObjectFinder<UTextureRenderTarget2D>(TEXT("/Script/Engine.TextureRenderTarget2D'/Game/Blueprints/InteractionSystem/RT_InspectionViewRenderTarget.RT_InspectionViewRenderTarget'")).Object;
	GetCaptureComponent2D()->TextureTarget = TextureRenderTarget2D;

	SetupPostProcessComponent();
	SetupCubeComponent();
	SetupRectLightComponent();
}

void AZWInteractionSceneCapture::SetupPostProcessComponent()
{
	PostProcessComponent->SetRelativeLocation(FVector(-163, 0, 0));
	PostProcessComponent->Settings.WhiteTemp = 9416;
}

void AZWInteractionSceneCapture::SetupCubeComponent()
{
	// Load the Cube mesh
	UStaticMesh* cubeMesh = ConstructorHelpers::FObjectFinder<UStaticMesh>(TEXT("StaticMesh'/Engine/BasicShapes/Cube.Cube'")).Object;

	// Set the component's mesh
	CubeComponent->SetStaticMesh(cubeMesh);

	CubeComponent->SetRelativeScale3D(FVector(5.8125, 5.8125, 5.8125));

	UMaterial* Material = ConstructorHelpers::FObjectFinder<UMaterial>(TEXT("Material'/Engine/BasicShapes/BasicShapeMaterial.BasicShapeMaterial'")).Object;

	CubeComponent->SetMaterial(0, Material);
}

void AZWInteractionSceneCapture::SetupRectLightComponent()
{
	RectLightComponent->SetRelativeLocation(FVector(-236, 0, 0));
	RectLightComponent->Intensity = 10000;
	RectLightComponent->IntensityUnits = ELightUnits::Unitless;
	RectLightComponent->SourceWidth = 496;
	RectLightComponent->SourceHeight = 456;
}

void AZWInteractionSceneCapture::SetupInspectedActor(AActor* InputActor)
{
	if (IsValid(InspectedActor))
	{
		InspectedActor->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		GetCaptureComponent2D()->ShowOnlyActorComponents(nullptr, true);
	}

	InspectedActor = InputActor;
	GetCaptureComponent2D()->ShowOnlyActorComponents(InspectedActor, true);
	
	if (IsValid(InputActor))
	{
		InputActor->AttachToComponent(ItemComponent, FAttachmentTransformRules::SnapToTargetNotIncludingScale, NAME_None);
	}
}

void AZWInteractionSceneCapture::UpdateVisibility(AActor* InActor)
{
	if (InActor != nullptr)
	{
		
	}
	SetupInspectedActor(InActor);
}

void AZWInteractionSceneCapture::AddInspectedActorLocation(FVector Location)
{
	FVector CurrentLocation = InspectedActor->GetRootComponent()->GetRelativeLocation();

	if (CurrentLocation.X > 35 || CurrentLocation.X < -20) { return; }

	CurrentLocation += Location;
	InspectedActor->SetActorRelativeLocation(CurrentLocation);
}

void AZWInteractionSceneCapture::AddInspectedActorRotation(FRotator Rotation)
{
	if (IsValid(InspectedActor))
	{
		InspectedActor->AddActorLocalRotation(Rotation);
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("InspectedActor not found!"))
	}
}

// @TODO: Set correct rotation so the objects are displayed properly when inspected regardless of the pivot 
void AZWInteractionSceneCapture::SetLookAtRotation(FVector Location)
{
	InspectedActor->SetActorRelativeRotation(UKismetMathLibrary::MakeRotFromX(Location));
}

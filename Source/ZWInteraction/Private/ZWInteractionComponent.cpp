// Fill out your copyright notice in the Description page of Project Settings.


#include "ZWInteractionComponent.h"
#include "ZWInteractionSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Components/ArrowComponent.h"
#include "Modules/ModuleManager.h"
//#include "FlowComponent.h"

void UZWInteractionComponent::NotifyFlowGraph()
{
	//if (IsValid(FlowComponent) && bIsFlowNotifier)
	//{
	//	FlowComponent->NotifyGraph(NotifyTag);
	//}
}

// Sets default values for this component's properties
UZWInteractionComponent::UZWInteractionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	//FlowComponent = CreateDefaultSubobject<UFlowComponent>(TEXT("FlowComponent"));

	if (IsValid(GetOwner()))
	{
		InspectionArrowComponent = CreateDefaultSubobject<UArrowComponent>(TEXT("Arrow"));
		InspectionArrowComponent->ArrowSize = 0.5f;
		//InspectionArrowComponent->SetupAttachment(GetOwner()->GetRootComponent());
		//InspectionArrowComponent->RegisterComponent();
		//if (IsValid(GetOwner()->GetRootComponent()))
		{
		//	InspectionArrowComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Arrow"));
		//	InspectionArrowComponent->SetRelativeRotation(ArrowRotation);
		}	
	}	
	
	ActorGuid = FGuid::NewGuid();
}

void UZWInteractionComponent::Interact()
{
	NotifyFlowGraph();
	OnInteract.Broadcast();
}

void UZWInteractionComponent::Inspect()
{
	NotifyFlowGraph();
	InteractionSubsystem->StartInspection();
	InteractionSubsystem->AdjustInspectionRotation(InspectionRotationAdjustment);
	OnInspect.Broadcast();
}

void UZWInteractionComponent::Investigate()
{
	NotifyFlowGraph();
	InvestigationCameraComponent->SetRelativeRotation(InitialCameraRotation);
	InteractionSubsystem->ResolveInvestigation(GetOwner());
	OnInvestigate.Broadcast();
}

void UZWInteractionComponent::ToggleHighlight(bool IsHighlighted)
{
	if (StaticMeshComponent == nullptr || InteractionSubsystem == nullptr) { return; }

	if (IsInvestigationExclusive())
	{
		if (!InteractionSubsystem->IsPlayerInvestigating())
		{
			return;
		}

		InteractionSubsystem->SetInteractableObject(this);
		StaticMeshComponent->SetRenderCustomDepth(IsHighlighted);
		bIsHighlighted = IsHighlighted;

		return;
	}

	InteractionSubsystem->SetInteractableObject(this);
	StaticMeshComponent->SetRenderCustomDepth(IsHighlighted);
	bIsHighlighted = IsHighlighted;
}

void UZWInteractionComponent::BPToggleHighlight_Implementation(bool IsHighlighted)
{
	ToggleHighlight(IsHighlighted);
}

#if WITH_EDITOR
void UZWInteractionComponent::DestroyInteractiveActor()
{
	OnRegisterActor.Broadcast(ActorGuid);
	GetOwner()->Destroy();
}

AActor* UZWInteractionComponent::ResolveInvestigationParentActor()
{
	if (bUseParentForInvestigation)
	{
		return GetOwner()->GetAttachParentActor();
	}
	return InvestigationParentActor;
}

void UZWInteractionComponent::PostEditChangeProperty(FPropertyChangedEvent& e)
{
	FName PropertyName = (e.Property != NULL) ? e.Property->GetFName() : NAME_None;
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZWInteractionComponent, bIsInvestigatable))
	{		
		if (bIsInvestigatable)
		{
			CreateCamera();
		}
		else
		{
			RemoveCamera();
		}
	}
	
	if (PropertyName == GET_MEMBER_NAME_CHECKED(UZWInteractionComponent, bIsInspectable))
	{		
		if (bIsInspectable)
		{
			//CreateArrow();
		}
		else
		{
			//RemoveArrow();
		}
	}	

	Super::PostEditChangeProperty(e);
}

void UZWInteractionComponent::CreateCamera()
{
	if (!InvestigationCameraComponent)
	{
		InvestigationCameraComponent = NewObject<UCameraComponent>(this, UCameraComponent::StaticClass(), TEXT("Camera"));
		InvestigationCameraComponent->RegisterComponent();
		if (IsValid(GetOwner()->GetRootComponent()))
		{
			InvestigationCameraComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Camera"));
			InvestigationCameraComponent->SetRelativeLocation(CameraPosition);
			InvestigationCameraComponent->SetRelativeRotation(CameraRotation);
		}
	}
}

void UZWInteractionComponent::RemoveCamera()
{
	if (InvestigationCameraComponent != nullptr)
	{
		CameraPosition = InvestigationCameraComponent->GetRelativeLocation();
		CameraRotation = InvestigationCameraComponent->GetRelativeRotation();
		InvestigationCameraComponent->DestroyComponent();
		InvestigationCameraComponent = nullptr;
	}
}
void UZWInteractionComponent::CreateArrow()
{
	if (!InspectionArrowComponent)
	{
		InspectionArrowComponent = NewObject<UArrowComponent>(this, UArrowComponent::StaticClass(), TEXT("Arrow"));
		InspectionArrowComponent->ArrowSize = 0.5f;
		InspectionArrowComponent->RegisterComponent();
		if (IsValid(GetOwner()->GetRootComponent()))
		{
			InspectionArrowComponent->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform, TEXT("Arrow"));
			InspectionArrowComponent->SetRelativeRotation(ArrowRotation);
		}
	}
}

void UZWInteractionComponent::RemoveArrow()
{
	if (InspectionArrowComponent != nullptr)
	{
		ArrowRotation = InspectionArrowComponent->GetRelativeRotation();
		InspectionArrowComponent->DestroyComponent();
		InspectionArrowComponent = nullptr;
	}
}
#endif

UCameraComponent *UZWInteractionComponent::GetCameraComponent()
{
	return InvestigationCameraComponent;
}

FRotator UZWInteractionComponent::GetInspectionRotationAdjustment()
{
	return InspectionRotationAdjustment;
}

// Called when the game starts
void UZWInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	AActor* Owner = GetOwner();
	UActorComponent* ActorComponent = Owner->GetComponentByClass(UStaticMeshComponent::StaticClass());
	if (ActorComponent == nullptr)
	{
		ActorComponent = Owner->FindComponentByTag(UStaticMeshComponent::StaticClass(), FName(TEXT("MainInteractionMesh")));
	}
	StaticMeshComponent = Cast<UStaticMeshComponent>(ActorComponent);
	
	if (StaticMeshComponent == nullptr) { return; }
	
	StaticMeshComponent->SetCustomDepthStencilValue(1);

	StaticMeshComponent->SetCollisionResponseToChannel(ECollisionChannel::ECC_GameTraceChannel2, ECollisionResponse::ECR_Block);

	UGameInstance* GameInstance = GetOwner()->GetGameInstance();
	InteractionSubsystem = GameInstance->GetSubsystem<UZWInteractionSubsystem>();

	if (InvestigationCameraComponent)
	{
		InitialCameraRotation = InvestigationCameraComponent->GetRelativeRotation();
	}
}

// Called every frame
void UZWInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
	
}

void UZWInteractionComponent::DestroyComponent(bool bPromoteChildren)
{
	if (InvestigationCameraComponent)
	{
		InvestigationCameraComponent->DestroyComponent();
		InvestigationCameraComponent = nullptr;
	}

	if (InspectionArrowComponent)
	{
		InspectionArrowComponent->DestroyComponent();
		InspectionArrowComponent = nullptr;
	}

	//if (FlowComponent)
	//{
	//	FlowComponent->DestroyComponent();
	//	FlowComponent = nullptr;
	//}

	Super::DestroyComponent(bPromoteChildren);
}

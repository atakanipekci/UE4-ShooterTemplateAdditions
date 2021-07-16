// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterGame.h"
#include "SplineActor.h"

// Sets default values
ASplineActor::ASplineActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SplineComponent = CreateDefaultSubobject<USplineComponent>("Spline");
	if(SplineComponent)
	{
		SetRootComponent(SplineComponent);  
	}
}

void ASplineActor::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	/** Create an Example Spline for preview purposes */
	ClearNodes();
	for(int i = 0; i < NodeCount; i++)
	{
		AddNode(FVector{static_cast<float>(i*200),0,0});
	}
	UpdateSpline();
}

void ASplineActor::AddNode(const FVector& Position)
{
	if(SplineComponent == nullptr)
	{
		return;
	}

	SplineComponent->AddSplineWorldPoint(Position);
}

void ASplineActor::ClearNodes()
{
	if(SplineComponent == nullptr)
	{
		return;
	}

	SplineComponent->ClearSplinePoints(true);
}

void ASplineActor::UpdateSpline()
{
	if(SplineComponent == nullptr)
	{
		return;
	}

	for(int i = SplineComponent->GetNumChildrenComponents()-1; i >= 0; i--)
	{
		USceneComponent* ChildComp = SplineComponent->GetChildComponent(i);
		if(ChildComp)
		{
			ChildComp->DestroyComponent();
		}
	}

	if(SplineComponent && SplineMeshMap.Num() > 0)
	{
		FSplineMeshDetails* StartMeshDetails = nullptr;
		if(SplineMeshMap.Contains(ESplineMeshType::Start))
		{
			StartMeshDetails = SplineMeshMap.Find(ESplineMeshType::Start);  
		}
		
		FSplineMeshDetails* EndMeshDetails = nullptr;
		if(SplineMeshMap.Contains(ESplineMeshType::End))
		{
			EndMeshDetails = SplineMeshMap.Find(ESplineMeshType::End);
		}

		FSplineMeshDetails* DefaultMeshDetails = nullptr;
		if(SplineMeshMap.Contains(ESplineMeshType::Default))
		{
			DefaultMeshDetails = SplineMeshMap.Find(ESplineMeshType::Default);  
		}
		else
		{
			// exit if we don't have a default mesh to work with
			return;
		}

		if(DefaultMeshDetails == nullptr)
		{
			return;
		}

		int SplinePointCount = SplineComponent->GetNumberOfSplinePoints();
		UStaticMesh* StaticMesh = DefaultMeshDetails->Mesh;
		UMaterialInterface* Material = nullptr;
		ESplineMeshAxis::Type ForwardAxis = DefaultMeshDetails->ForwardAxis;

		for(int i = 0; i < SplinePointCount-1 ; i++)
		{
			USplineMeshComponent* SplineMesh = NewObject<USplineMeshComponent>(this, USplineMeshComponent::StaticClass());
			if(SplineMesh == nullptr)
			{
				continue;
			}
			// Start of spline
			if(StartMeshDetails && StartMeshDetails->Mesh && i == 0)
			{
				StaticMesh = StartMeshDetails->Mesh;
				ForwardAxis = StartMeshDetails->ForwardAxis;
				Material = StartMeshDetails->Material;        
			}
			// End of spline
			else if(EndMeshDetails && EndMeshDetails->Mesh && SplinePointCount > 2 && i == (SplinePointCount - 2))
			{
				StaticMesh = EndMeshDetails->Mesh;
				ForwardAxis = EndMeshDetails->ForwardAxis;
				Material = EndMeshDetails->Material;
			}
			//Middle/Default spline
			else
			{
				StaticMesh = DefaultMeshDetails->Mesh;
				ForwardAxis = DefaultMeshDetails->ForwardAxis;
				Material = DefaultMeshDetails->Material;
			}

			SplineMesh->SetStaticMesh(StaticMesh);
			SplineMesh->SetForwardAxis(ForwardAxis, true);
			SplineMesh->SetMaterial(0, Material);

			SplineComponent->SetSplinePointType(i,ESplinePointType::Linear,false);

			SplineComponent->SetTangentsAtSplinePoint(i, FVector::ZeroVector, FVector::ZeroVector, ESplineCoordinateSpace::Local);


			SplineMesh->RegisterComponent();
			SplineMesh->CreationMethod = EComponentCreationMethod::UserConstructionScript;
			SplineMesh->SetMobility(EComponentMobility::Movable);
			SplineMesh->AttachToComponent(SplineComponent, FAttachmentTransformRules::KeepRelativeTransform);
			const FVector StartPoint = SplineComponent->GetLocationAtSplinePoint(i, ESplineCoordinateSpace::Type::Local);
			const FVector StartTangent = SplineComponent->GetTangentAtSplinePoint(i, ESplineCoordinateSpace::Type::Local);
			const FVector EndPoint = SplineComponent->GetLocationAtSplinePoint(i + 1, ESplineCoordinateSpace::Type::Local);
			const FVector EndTangent = SplineComponent->GetTangentAtSplinePoint(i + 1, ESplineCoordinateSpace::Type::Local);
			SplineMesh->SetStartAndEnd(StartPoint, StartTangent, EndPoint, EndTangent, true);

			
			SplineMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}

		SplineComponent->UpdateSpline();
	}
}


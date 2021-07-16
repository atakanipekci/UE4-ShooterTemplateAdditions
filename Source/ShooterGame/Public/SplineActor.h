// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "Components/SplineMeshComponent.h"
#include "GameFramework/Actor.h"
#include "SplineActor.generated.h"

UENUM(BlueprintType)
enum class ESplineMeshType: uint8 {
  Default    UMETA(DisplayName = "Default Mesh"),
  Start    UMETA(DisplayName = "Starting Mesh"),
  End      UMETA(DisplayName = "Ending Mesh"),
};

USTRUCT(BlueprintType)
struct FSplineMeshDetails : public FTableRowBase
{
  GENERATED_BODY()
  
  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  UStaticMesh* Mesh;
  UPROPERTY(EditAnywhere, BlueprintReadOnly)
  TEnumAsByte<ESplineMeshAxis::Type> ForwardAxis;
  
  UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
  class UMaterialInterface* Material;
  FSplineMeshDetails() : ForwardAxis(ESplineMeshAxis::Type::X)
  {
  }
};

UCLASS()
class SHOOTERGAME_API ASplineActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ASplineActor();
	virtual void OnConstruction(const FTransform& Transform) override;
	/** Add a new node to the spline component of this actor */
	void AddNode(const FVector& Position);
	/** Clear all nodes from the spline component */
	void ClearNodes();
	/** Refresh the spline and add the spline meshes at the corresponding spline points */
	void UpdateSpline();

	/** Actual Spline component that this class provides functionalities for */
	UPROPERTY(VisibleAnywhere, Category = "Spline")
	USplineComponent* SplineComponent;

	/** Map data that is used to visualize the spline with correct mesh and materials */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Spline")
	TMap<ESplineMeshType, FSplineMeshDetails> SplineMeshMap;

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category = "Spline")
	int NodeCount = 3;
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PathfindingStructs.h"
#include "JPSGridActor.generated.h"

USTRUCT()
struct FJPSCell
{
  GENERATED_USTRUCT_BODY()

  FVector position;
  FVector gridIndex;
  bool walkable;

  short f, g, h;
  EListStatus cellType;

  FJPSCell* parentCell;
  FJPSCell* neighbors[8];
};

USTRUCT()
struct FJPSPathfindingPacket
{
  GENERATED_USTRUCT_BODY()

  UActorComponent* requestingActor;
  FJPSCell* startCell;
  FJPSCell* targetCell;
};


UCLASS()
class FEATUREPROJECT_API AJPSGridActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJPSGridActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  void ExtendGrid(FVector a_newGridSize);
  void ExtendGrid(FVector a_newGridSize, FVector a_newGridOffset);

  FJPSCell* GetCellByPosition(FVector a_position);
  FJPSCell* GetCellByIndex(FVector a_index);

  FVector GetGridCellSize() { return m_gridCellSize; } // GridCellSize is in world units
  FVector GetGrifOffSet() { return m_gridOffSet; } // Offset is defined in the amount of cells
  FVector GetGridSize() { return m_gridSize; } // Returns the size in #of cells per axis

private:
  void DrawDebug();

  void AddNeighbors(FJPSCell &a_Cell);
  TArray<FJPSCell> m_grid;

  void GenerateGrid();

  UPROPERTY(EditAnywhere)
    FVector m_gridCellSize;
  UPROPERTY(EditAnywhere)
    FVector m_gridOffSet;
  UPROPERTY(EditAnywhere)
    FVector m_gridSize;


  UPROPERTY(EditAnywhere)
    bool m_drawDebug;	
};

// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PathfindingStructs.h"
#include "JPSPlusGridActor.generated.h"

USTRUCT()
struct FPathfindingCell
{
  GENERATED_USTRUCT_BODY()

  FVector position;
  FVector gridIndex;
  bool walkable;

  short distanceToJumppoint[8];

  short f, g, h;
  EListStatus cellType;
  FPathfindingCell* parentCell;
};

USTRUCT()
struct FJPSPlusPathfindingPacket
{
  GENERATED_USTRUCT_BODY()

    UActorComponent* requestingActor;
  FPathfindingCell* startCell;
  FPathfindingCell* targetCell;
};


USTRUCT()
struct FPrimaryJumpPoint
{
  GENERATED_USTRUCT_BODY()

    TArray<FVector> jumpDirections;
  bool isjumpPoint;
};

UCLASS()
class FEATUREPROJECT_API AJPSPlusGridActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJPSPlusGridActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  void GenerateGrid();

  FPathfindingCell* GetCellByPosition(FVector a_position);
  FPathfindingCell* GetCellByIndex(FVector a_index);

  FVector GetGridSize() { return m_gridSize; }

private:
  void PrecalculateJumpPoints();
  bool TestStraightForcedNeighbour(FPathfindingCell* a_origCell, FPathfindingCell* a_directioncell, FVector a_direction, FPrimaryJumpPoint& a_jp);
  bool TestDiagonalForcedNeighbour(FPathfindingCell* a_origCell, FPathfindingCell* a_directioncell, FVector a_direction);

  short JumpStraight(FPathfindingCell* a_Cell, FVector a_direction);
  short JumpDiagonal(FPathfindingCell* a_Cell, FVector a_direction);
  bool IsCellJumpPoint(FVector a_index);
  bool IsCellValidStraightJumpPoint(FPathfindingCell* a_Cell, FVector a_direction);

  void DrawDebug();
  FColor GetDebugColor(short a_dist);

  TArray<FPathfindingCell> m_grid;

  UPROPERTY(EditAnywhere)
    bool m_drawPrimaryJumpPoints;
  UPROPERTY(EditAnywhere)
    bool m_drawGrid;
  UPROPERTY(EditAnywhere)
    bool m_drawStraightDistanceToJumpPoints;
  UPROPERTY(EditAnywhere)
    bool m_drawDiagonalDistanceToJumpPoints;

  UPROPERTY(EditAnywhere)
    FVector m_gridSize;
  UPROPERTY(EditAnywhere)
    FVector m_gridOffset;
  UPROPERTY(EditAnywhere)
    FVector m_cellSize;

  TArray<FPrimaryJumpPoint> JumpPointGrid;
};

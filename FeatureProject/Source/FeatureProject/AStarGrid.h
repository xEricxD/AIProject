//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "PathfindingStructs.h"
#include "AStarGrid.generated.h"

USTRUCT()
struct FAStarCell
{
  GENERATED_USTRUCT_BODY()

  FVector position;
  FVector gridIndex;
  bool walkable;

  short f, g, h;
  EListStatus cellType;

  FAStarCell* parentCell;
  FAStarCell* neighbors[8];
};

USTRUCT()
struct FAStarPathfindingPacket
{
  GENERATED_USTRUCT_BODY()

  UActorComponent* requestingActor;
  FAStarCell* startCell;
  FAStarCell* targetCell;
};

UCLASS()
class FEATUREPROJECT_API AAStarGrid : public AActor
{
	GENERATED_BODY()
	
public:	
	//Sets default values for this actor's properties
	AAStarGrid();

	//Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  //functions for changing grid size
  void ExtendGrid(FVector a_newGridSize);
  void ExtendGrid(FVector a_newGridSize, FVector a_newGridOffset);

  FAStarCell* GetCellByPosition(FVector a_position);
  FAStarCell* GetCellByIndex(FVector a_index);

  FVector GetGridCellSize() { return m_gridCellSize; } //GridCellSize is in world units
  FVector GetGrifOffSet() { return m_gridOffSet; } //Offset is defined in the amount of cells
  FVector GetGridSize() { return m_gridSize; } //Returns the size in #of cells per axis

private:
  void DrawDebug();

  //calculate the neighbors of this cell
  void AddNeighbors(FAStarCell &a_Cell);
  TArray<FAStarCell> m_grid;

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

// Fill out your copyright notice in the Description page of Project Settings.

#include "FeatureProject.h"
#include "JPSGridActor.h"


// Sets default values
AJPSGridActor::AJPSGridActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJPSGridActor::BeginPlay()
{
	Super::BeginPlay();

  GenerateGrid();

  int gridSize = m_gridSize.X * m_gridSize.Y * m_gridSize.Z;
  for (int i = 0; i < gridSize; i++)
    AddNeighbors(m_grid[i]);
}

void AJPSGridActor::GenerateGrid()
{
  if (m_gridSize.X == 0 || m_gridSize.Y == 0 || m_gridSize.Z == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to generate pathfinding grid!, PathfindingGridComponent.cpp"));
    return;
  }

  m_grid.Empty();

  int gridSize = m_gridSize.X * m_gridSize.Y * m_gridSize.Z;
  FVector startPos = (FVector(m_gridOffSet.X * m_gridCellSize.X, m_gridOffSet.Y * m_gridCellSize.Y, (m_gridOffSet.Z * m_gridCellSize.Z) + (m_gridCellSize.Z / 2.0f)));

  FVector cellPos;
  FVector counter(0, 0, 0);

  FVector rowOffset(0, 0, 0);
  FCollisionShape collisionShape;
  collisionShape.MakeBox(m_gridCellSize * 0.98f);
  collisionShape.Box.HalfExtentX = (m_gridCellSize.X * 0.98f * 0.5f);
  collisionShape.Box.HalfExtentY = (m_gridCellSize.Y * 0.98f * 0.5f);
  collisionShape.Box.HalfExtentZ = (m_gridCellSize.Z * 0.98f * 0.5f);
  UWorld* world = GetWorld();
  FQuat rotation;
  FName profileName("BlockAll");
  FCollisionQueryParams params;

  int col = 0;

  for (int i = 0; i < gridSize; i++)
  {
    //Calculate cell position
    cellPos.X = startPos.X + counter.X * m_gridCellSize.X;
    cellPos.Y = startPos.Y + counter.Y * m_gridCellSize.Y;
    cellPos.Z = startPos.Z + counter.Z * m_gridCellSize.Z;

    FJPSCell newCell;
    newCell.position = cellPos;
    newCell.gridIndex = counter;
    newCell.h = newCell.f = newCell.g = 0;
    newCell.cellType = EListStatus::NONE;
    newCell.parentCell = nullptr;
    newCell.walkable = true;

    for (int j = 0; j < 8; j++)
      newCell.neighbors[j] = nullptr;

    // Do Collision Test
    FVector position = cellPos;
    if (world->OverlapBlockingTestByProfile(position, rotation, profileName, collisionShape, params))
    {
      col++;
      newCell.walkable = false;
    }

    m_grid.Add(newCell);

    if (counter.X < m_gridSize.X - 1)
      counter.X++;
    else if (counter.Y < m_gridSize.Y - 1)
    {
      counter.Y++;
      counter.X = 0;
    }
    else if (counter.Z < m_gridSize.Z - 1)
    {
      counter.Z++;
      counter.Y = 0;
      counter.X = 0;
    }
  }
}

void AJPSGridActor::AddNeighbors(FJPSCell &a_cell)
{
  //if the cell is not walkable, we don't need its neighbors
  if (!a_cell.walkable)
    return;

  //left cell
  FJPSCell* c = GetCellByIndex(a_cell.gridIndex + FVector(-1, 0, 0));
  if (c && c->walkable)
    a_cell.neighbors[0] = c;
  //right cell
  c = GetCellByIndex(a_cell.gridIndex + FVector(1, 0, 0));
  if (c && c->walkable)
    a_cell.neighbors[1] = c;
  //top cell
  c = GetCellByIndex(a_cell.gridIndex + FVector(0, -1, 0));
  if (c && c->walkable)
    a_cell.neighbors[2] = c;
  //bottom cell
  c = GetCellByIndex(a_cell.gridIndex + FVector(0, 1, 0));
  if (c && c->walkable)
    a_cell.neighbors[3] = c;

  //now add diagonals
  //left - top
  //if top or left cell is not a neighbor yet, we can't move to the diagonal because one of those nodes is unavailable
  //if (a_cell.neighbors[0] && a_cell.neighbors[2])
  {
    c = GetCellByIndex(a_cell.gridIndex + FVector(-1, -1, 0));
    if (c && c->walkable)
      a_cell.neighbors[4] = c;
  }
  //left - bottom
  //if (a_cell.neighbors[0] && a_cell.neighbors[3])
  {
    c = GetCellByIndex(a_cell.gridIndex + FVector(-1, 1, 0));
    if (c && c->walkable)
      a_cell.neighbors[5] = c;
  }
  //right - top
  //if (a_cell.neighbors[1] && a_cell.neighbors[2])
  {
    c = GetCellByIndex(a_cell.gridIndex + FVector(1, -1, 0));
    if (c && c->walkable)
      a_cell.neighbors[6] = c;
  }
  //right - bottom
  //if (a_cell.neighbors[1] && a_cell.neighbors[3])
  {
    c = GetCellByIndex(a_cell.gridIndex + FVector(1, 1, 0));
    if (c && c->walkable)
      a_cell.neighbors[7] = c;
  }
}

void AJPSGridActor::ExtendGrid(FVector a_newGridSize)
{
  m_gridSize = a_newGridSize;
  m_grid.Empty();
  GenerateGrid();
}

void AJPSGridActor::ExtendGrid(FVector a_newGridSize, FVector a_newGridOffset)
{
  m_gridSize = a_newGridSize;
  m_gridOffSet = a_newGridOffset;
  m_grid.Empty();
  GenerateGrid();
}

// Called every frame
void AJPSGridActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

  if (m_drawDebug)
    DrawDebug();
}

FJPSCell* AJPSGridActor::GetCellByPosition(FVector a_position)
{
  FVector startPosOfGrid = (FVector(m_gridOffSet.X * m_gridCellSize.X, m_gridOffSet.Y * m_gridCellSize.Y, m_gridOffSet.Z * m_gridCellSize.Z));
  FVector offset = -startPosOfGrid + a_position;

  int xIndex = FMath::RoundToInt(offset.X / m_gridCellSize.X);
  int yIndex = FMath::RoundToInt(offset.Y / m_gridCellSize.Y);
  int zIndex = 0;// We're not using height FMath::RoundToInt(offset.Z / m_gridCellSize.Z);

  return GetCellByIndex(FVector(xIndex, yIndex, zIndex));
}

FJPSCell* AJPSGridActor::GetCellByIndex(FVector a_index)
{
  //just do a check if the index asked for is within grid bounds and then return
  if (a_index.X < 0 || a_index.Y < 0 || a_index.Z < 0)
    return nullptr;

  if (a_index.X > m_gridSize.X - 1 || a_index.Y > m_gridSize.Y - 1 || a_index.Z > m_gridSize.Z - 1)
    return nullptr;

  return &m_grid[a_index.X + a_index.Y * m_gridSize.X + (a_index.Z * m_gridSize.X * m_gridSize.Y)];
}

void AJPSGridActor::DrawDebug()
{
  for (FJPSCell &cell : m_grid)
  {
    FColor col = (cell.walkable) ? FColor::Yellow : FColor::Red;
    FVector size = FVector(m_gridCellSize.X, m_gridCellSize.Y, m_gridCellSize.Z) * 0.5f;
    size.Z = 0;
    DrawDebugBox(GetWorld(), cell.position, size * 0.8f, col);
  }
}

// Fill out your copyright notice in the Description page of Project Settings.

#include "FeatureProject.h"
#include "JPSPlusGridActor.h"

// Sets default values
AJPSPlusGridActor::AJPSPlusGridActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJPSPlusGridActor::BeginPlay()
{
	Super::BeginPlay();

  //start of by generating a new grid
  GenerateGrid();
  PrecalculateJumpPoints();
	
}

// Called every frame
void AJPSPlusGridActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

  DrawDebug();
}

void AJPSPlusGridActor::GenerateGrid()
{
  if (m_gridSize.X == 0 || m_gridSize.Y == 0 || m_gridSize.Z == 0)
  {
    UE_LOG(LogTemp, Error, TEXT("Failed to generate pathfinding grid!, PathfindingGridComponent.cpp"));
    return;
  }

  m_grid.Empty();

  int gridSize = m_gridSize.X * m_gridSize.Y * m_gridSize.Z;
  FVector startPos = (FVector(m_gridOffset.X * m_cellSize.X, m_gridOffset.Y * m_cellSize.Y, (m_gridOffset.Z * m_cellSize.Z) + (m_cellSize.Z / 2.0f)));

  FVector cellPos;
  FVector counter(0, 0, 0);

  FVector rowOffset(0, 0, 0);
  FCollisionShape collisionShape;
  collisionShape.MakeBox(m_cellSize * 0.98f);
  collisionShape.Box.HalfExtentX = (m_cellSize.X * 0.98f * 0.5f);
  collisionShape.Box.HalfExtentY = (m_cellSize.Y * 0.98f * 0.5f);
  collisionShape.Box.HalfExtentZ = (m_cellSize.Z * 0.98f * 0.5f);
  UWorld* world = GetWorld();
  FQuat rotation;
  FName profileName("BlockAll");
  FCollisionQueryParams params;

  int col = 0;

  for (int i = 0; i < gridSize; i++)
  {
    //Calculate cell position
    cellPos.X = startPos.X + counter.X * m_cellSize.X;
    cellPos.Y = startPos.Y + counter.Y * m_cellSize.Y;
    cellPos.Z = startPos.Z + counter.Z * m_cellSize.Z;

    FPathfindingCell newCell;
    newCell.position = cellPos;
    newCell.gridIndex = counter;
    newCell.h = newCell.f = newCell.g = 0;
    newCell.cellType = EListStatus::NONE;
    newCell.parentCell = nullptr;
    newCell.walkable = true;
    for (int j = 0; j < 8; j++)
      newCell.distanceToJumppoint[j] = 0;

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

void AJPSPlusGridActor::PrecalculateJumpPoints()
{
  //create a grid that specifies where the jump points are, and in what directions they are jump points
  JumpPointGrid.SetNum(m_grid.Num());

  for (int i = 0; i < m_grid.Num(); i++)
  {
    // for every cell on the grid, test if the cell is a forced neighbour for any of its directions (O(n) compute)
    FPrimaryJumpPoint newJP;
    newJP.isjumpPoint = false;
    for (int j = -1; j < 2; j++)
    {
      for (int k = -1; k < 2; k++)
      {
        if (j == 0 && k == 0)
          continue;

        if (j != 0 && k != 0) //only test straights
          continue;

        FPathfindingCell* c = GetCellByIndex(m_grid[i].gridIndex + FVector(k, j, 0));
        if (c && c->walkable) //if c isnt walkable, this can be a forced neighbour from this side
        {
          if (TestStraightForcedNeighbour(&m_grid[i], c, FVector(k, j, 0), newJP))
            newJP.isjumpPoint = true;
        }
      }
    }

    JumpPointGrid[i] = newJP;
  }

  // Now that we have all primary jump points, let's calculate the straight jump points
  for (int i = 0; i < m_grid.Num(); i++)
  {
    FPathfindingCell* c = &m_grid[i];
    if (c && c->walkable)
    {
      // go in 4 straight directions, and jump until we hit a wall, or a primary jump point
      c->distanceToJumppoint[1] = JumpStraight(c, FVector(0, -1, 0));
      c->distanceToJumppoint[3] = JumpStraight(c, FVector(-1, 0, 0));
      c->distanceToJumppoint[4] = JumpStraight(c, FVector(1, 0, 0));
      c->distanceToJumppoint[6] = JumpStraight(c, FVector(0, 1, 0));
    }
  }

  //And finally precalculate the diagonal jump points
  for (int i = 0; i < m_grid.Num(); i++)
  {
    FPathfindingCell* c = &m_grid[i];
    if (c && c->walkable)
    {
      // go in 4 diagonal directions, and jump until we hit a wall, or a straight jump point
      c->distanceToJumppoint[0] = JumpDiagonal(c, FVector(-1, -1, 0));
      c->distanceToJumppoint[2] = JumpDiagonal(c, FVector(1, -1, 0));
      c->distanceToJumppoint[5] = JumpDiagonal(c, FVector(-1, 1, 0));
      c->distanceToJumppoint[7] = JumpDiagonal(c, FVector(1, 1, 0));
    }
  }
}

bool AJPSPlusGridActor::TestStraightForcedNeighbour(FPathfindingCell* a_origCell, FPathfindingCell* a_directioncell, FVector a_direction, FPrimaryJumpPoint& a_jp)
{
  //test for unwalkable cells that would make a_cell a forced neighbour
  if (a_direction.Y == 0) // moving on x axis
  {
    FPathfindingCell* c = GetCellByIndex(a_directioncell->gridIndex + FVector(0, -1, 0)); //test top cell
    if (!c || !c->walkable)
    {
      c = GetCellByIndex(a_origCell->gridIndex + FVector(0, -1, 0));
      if (c && c->walkable)
      {
        a_jp.jumpDirections.Add(a_direction);
        return true;
      }
    }
    c = GetCellByIndex(a_directioncell->gridIndex + FVector(0, 1, 0)); //test bottom cell
    if (!c || !c->walkable)
    {
      c = GetCellByIndex(a_origCell->gridIndex + FVector(0, 1, 0));
      if (c && c->walkable)
      {
        a_jp.jumpDirections.Add(a_direction);
        return true;
      }
    }
  }
  else
  {
    FPathfindingCell* c = GetCellByIndex(a_directioncell->gridIndex + FVector(-1, 0, 0)); //test left cell
    if (!c || !c->walkable)
    {
      c = GetCellByIndex(a_origCell->gridIndex + FVector(-1, 0, 0));
      if (c && c->walkable)
      {
        a_jp.jumpDirections.Add(a_direction);
        return true;
      }
    }
    c = GetCellByIndex(a_directioncell->gridIndex + FVector(1, 0, 0)); //test right cell
    if (!c || !c->walkable)
    {
      c = GetCellByIndex(a_origCell->gridIndex + FVector(1, 0, 0));
      if (c && c->walkable)
      {
        a_jp.jumpDirections.Add(a_direction);
        return true;
      }
    }
  }

  return false;
}

bool AJPSPlusGridActor::TestDiagonalForcedNeighbour(FPathfindingCell* a_origCell, FPathfindingCell* a_directioncell, FVector a_direction)
{
  return false;
}

short AJPSPlusGridActor::JumpStraight(FPathfindingCell* a_Cell, FVector a_direction)
{
  FVector index = a_Cell->gridIndex + a_direction;
  FPathfindingCell* jump = GetCellByIndex(index);

  short dist = 0;
  while (jump && jump->walkable)
  {
    if (IsCellJumpPoint(jump->gridIndex))
    {
      //the cell is a jump point, but is it a jumppoint in the right direction?
      FPrimaryJumpPoint jp = JumpPointGrid[jump->gridIndex.X + jump->gridIndex.Y * m_gridSize.X + (jump->gridIndex.Z * m_gridSize.X * m_gridSize.Y)];
      for (FVector dir : jp.jumpDirections)
      {
        if (dir == -a_direction)
          return (dist + 1); //we found a jump point, return the distance to it
      }
    }

    jump = GetCellByIndex(jump->gridIndex + a_direction);
    dist++;
  }

  //return negative dist -> we hit a wall
  return -dist;
}

// TODO - fix diagonal wall avoidance (currently moves diagonal over walls)
short AJPSPlusGridActor::JumpDiagonal(FPathfindingCell* a_Cell, FVector a_direction)
{
  FVector index = a_Cell->gridIndex + a_direction;
  FPathfindingCell* jump = GetCellByIndex(index);

  short dist = 0;
  while (jump && jump->walkable)
  {
    //check if there's a straight jump node going in the correct direction(s)
    if (IsCellValidStraightJumpPoint(jump, a_direction))
      return (dist + 1);

    jump = GetCellByIndex(jump->gridIndex + a_direction);
    dist++;
  }

  //return negative dist -> we hit a wall
  return -dist;
}

bool AJPSPlusGridActor::IsCellJumpPoint(FVector a_index)
{
  if (a_index.X < 0 || a_index.Y < 0 || a_index.Z < 0)
    return false;
  if (a_index.X > m_gridSize.X - 1 || a_index.Y > m_gridSize.Y - 1 || a_index.Z > m_gridSize.Z - 1)
    return false;

  return JumpPointGrid[a_index.X + a_index.Y * m_gridSize.X + (a_index.Z * m_gridSize.X * m_gridSize.Y)].isjumpPoint;
}

bool AJPSPlusGridActor::IsCellValidStraightJumpPoint(FPathfindingCell* a_Cell, FVector a_direction)
{
  //test if left / right straight jump points point towards valid primary jump point
  if (a_direction.X == 1)
  {
    if (a_Cell->distanceToJumppoint[4] > 0)
      return true;
  }
  else if (a_Cell->distanceToJumppoint[3] > 0)
    return true;

  //test if up / down ""
  if (a_direction.Y == 1)
  {
    if (a_Cell->distanceToJumppoint[6] > 0)
      return true;
  }
  else if (a_Cell->distanceToJumppoint[1] > 0)
    return true;

  return false;
}

FPathfindingCell* AJPSPlusGridActor::GetCellByPosition(FVector a_position)
{
  FVector startPosOfGrid = (FVector(m_gridOffset.X * m_cellSize.X, m_gridOffset.Y * m_cellSize.Y, m_gridOffset.Z * m_cellSize.Z));
  FVector offset = -startPosOfGrid + a_position;

  int xIndex = FMath::RoundToInt(offset.X / m_cellSize.X);
  int yIndex = FMath::RoundToInt(offset.Y / m_cellSize.Y);
  int zIndex = 0;// We're not using height FMath::RoundToInt(offset.Z / m_cellSize.Z);

  return GetCellByIndex(FVector(xIndex, yIndex, zIndex));
}

FPathfindingCell* AJPSPlusGridActor::GetCellByIndex(FVector a_index)
{
  //just do a check if the index asked for is within grid bounds and then return
  if (a_index.X < 0 || a_index.Y < 0 || a_index.Z < 0)
    return nullptr;

  if (a_index.X > m_gridSize.X - 1 || a_index.Y > m_gridSize.Y - 1 || a_index.Z > m_gridSize.Z - 1)
    return nullptr;

  return &m_grid[a_index.X + a_index.Y * m_gridSize.X + (a_index.Z * m_gridSize.X * m_gridSize.Y)];
}

void AJPSPlusGridActor::DrawDebug()
{
  int count = 0;
  for (FPathfindingCell &cell : m_grid)
  {
    if (m_drawGrid)
    {
      FColor col = (cell.walkable) ? FColor::Green : FColor::Red;
      FVector size = FVector(m_cellSize.X, m_cellSize.Y, m_cellSize.Z) * 0.5f;
      size.Z = 0;
      DrawDebugBox(GetWorld(), cell.position, size * 0.8f, col);
    }

    if (m_drawPrimaryJumpPoints)
    {
      if (JumpPointGrid[count].isjumpPoint)
      {
        DrawDebugSphere(GetWorld(), cell.position, 25, 8, FColor::Magenta);
        for (FVector dir : JumpPointGrid[count].jumpDirections)
          DrawDebugSphere(GetWorld(), cell.position + (dir * 25), 12, 4, FColor::Yellow);
      }
    }
    count++;

    if (m_drawStraightDistanceToJumpPoints || m_drawDiagonalDistanceToJumpPoints)
    {
      //draw jump distances
      if (cell.walkable)
      {
        int c = 0;
        for (int i = -1; i < 2; i++)
        {
          for (int j = -1; j < 2; j++)
          {
            if (j == 0 && i == 0)
              continue;
            if ((j == 0 || i == 0) && m_drawStraightDistanceToJumpPoints)
            {
              FColor col = GetDebugColor(cell.distanceToJumppoint[c]);
              DrawDebugSphere(GetWorld(), cell.position + (FVector(j, i, 0) * 20), 12, 4, col);
            }
            if ((j != 0 && i != 0) && m_drawDiagonalDistanceToJumpPoints)
            {
              FColor col = GetDebugColor(cell.distanceToJumppoint[c]);
              DrawDebugSphere(GetWorld(), cell.position + (FVector(j, i, 0) * 20), 12, 4, col);
            }
            c++;
          }
        }
      }
    }
  }
}

FColor AJPSPlusGridActor::GetDebugColor(short a_dist)
{
  FColor col;
  if (a_dist <= 0)
  {
    col.R = col.G = col.B = 255 - (30 * a_dist);
  }
  else
  {
    col = FColor::Blue;
    col.G = 255 - (30 * a_dist);
  }
  return col;
}


//Fill out your copyright notice in the Description page of Project Settings.
//references
//http://zerowidth.com/2013/05/05/jump-point-search-explained.html
//http://grastien.net/ban/articles/hg-aaai11.pdf

#include "FeatureProject.h"
#include "JPSAgentComponent.h"
#include "JPSPathfinder.h"

//Sets default values
AJPSPathfinderActor::AJPSPathfinderActor()
{
 	//Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

//Called when the game starts or when spawned
void AJPSPathfinderActor::BeginPlay()
{
	Super::BeginPlay();

  for (TActorIterator<AJPSGridActor> Itr(GetWorld()); Itr; ++Itr)
    m_grid = *Itr;

  if (!m_grid)
    UE_LOG(LogTemp, Error, TEXT("ERROR: could not find grid, JPSPathfinder.cpp"));
}

void AJPSPathfinderActor::RequestPath(UActorComponent* a_requester, FJPSCell* a_startCell, FJPSCell* a_targetCell)
{
  FJPSPathfindingPacket newPacket;
  newPacket.requestingActor = a_requester;
  newPacket.startCell = a_startCell;
  newPacket.targetCell = a_targetCell;

  m_pathQueue.Enqueue(newPacket);
}

//Called every frame
void AJPSPathfinderActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

  if (!m_pathQueue.IsEmpty()) //the queue is not empty, there is an actor requesting a path
  {
    FJPSPathfindingPacket currentPacket;
    m_pathQueue.Peek(currentPacket);

    AddStartCell(currentPacket);

    bool res = SearchLoop(currentPacket);

    if (res)
    {
      FPath* path = new FPath();
      CreatePath(currentPacket.targetCell, path);
      ((UJPSAgentComponent*)(currentPacket.requestingActor))->SetPath(path);
      m_pathQueue.Dequeue(currentPacket);
      m_isFindingPath = false;
      Clear();
    }
  }

  if (m_debugdraw)
    DrawDebug();
}


void AJPSPathfinderActor::AddStartCell(FJPSPathfindingPacket &a_packet)
{
  if (!m_isFindingPath)
  {
    a_packet.startCell->parentCell = nullptr;
    a_packet.startCell->cellType = EListStatus::ON_OPEN;
    m_goalCell = a_packet.targetCell;
    m_openList.Add(a_packet.startCell);
    m_isFindingPath = true;
  }
}

bool AJPSPathfinderActor::SearchLoop(FJPSPathfindingPacket &a_packet)
{
  bool targetFound = false;
  while (!targetFound)
  {
    if (m_updateStepBased && !m_debugStep)
      break;

    if (m_openList.Num() == 0)
    {
      //no valid path can be found
      m_pathQueue.Dequeue(a_packet); //remove the requester from the queue since path is invalid
      m_isFindingPath = false;
      Clear();
      return false;
    }

    FJPSCell* cell = m_openList[0];

    //get the node with the lowest f value of the open list
    for (FJPSCell* c : m_openList)
    {
      if (cell->f > c->f)
        cell = c;
    }

    //remove lowest f node from open list and add to closed list
    m_openList.Remove(cell);

    if (cell != a_packet.targetCell)
    {
      AddNeighbors(cell);

      m_closedList.Add(cell);
      cell->cellType = EListStatus::ON_CLOSED;
    }
    else
      targetFound = true;

    m_debugStep = false;
  }

  return targetFound;
}

void AJPSPathfinderActor::CreatePath(FJPSCell* a_cell, FPath* a_path)
{
  //backtrace the path to the starting node
  a_path->path.Add(a_cell->position);
  if (a_cell->parentCell)
  {
    CreatePath(a_cell->parentCell, a_path);
  }
}

void AJPSPathfinderActor::AddNeighbors(FJPSCell* a_cell)
{
  //start off by pruning the neighbors (removing any neighbors that do not need examining)
  TArray<FJPSCell*> neighbors;
  if (a_cell->parentCell)
  {
    FVector dir = a_cell->gridIndex - a_cell->parentCell->gridIndex;
    dir.X = FMath::Clamp<float>(dir.X, -1, 1);
    dir.Y = FMath::Clamp<float>(dir.Y, -1, 1);

    //test if we're jumping diagonal or straight
    if (dir.X != 0 && dir.Y != 0)
      neighbors = PruneDiagonal(a_cell, dir);
    else
      neighbors = PruneStraight(a_cell, dir);
  }
  else //no parent cell -> dont prune any neighbors
  {
    for (FJPSCell* neighbor : a_cell->neighbors)
    {
      if (neighbor && neighbor->walkable) //make sure neighbor is available
      {
        neighbor->parentCell = a_cell;
        neighbors.Add(neighbor);
      }
    }
  }

  //Now that we pruned all the neighbors we don't need, try to jump for each neighbor we do need to check
  for (FJPSCell* neighbor : neighbors)
  {
    FVector dir = neighbor->gridIndex - neighbor->parentCell->gridIndex;
    dir.X = FMath::Clamp<float>(dir.X, -1, 1);
    dir.Y = FMath::Clamp<float>(dir.Y, -1, 1);

    FJPSPathfindingPacket current;
    m_pathQueue.Peek(current);
    FJPSCell* jumpPoint = FindJumpPoint(neighbor->parentCell, dir, current.startCell, current.targetCell);
    if (jumpPoint && jumpPoint->cellType == EListStatus::NONE)
    {
      //if we find a jump point, give it all its values and add it to the open list, as with Astar
      jumpPoint->g = a_cell->g + GetGValueJPS(jumpPoint, a_cell);
      jumpPoint->cellType = EListStatus::ON_OPEN;
      jumpPoint->parentCell = neighbor->parentCell;
      jumpPoint->h = GetHeuristic(jumpPoint);
      jumpPoint->f = jumpPoint->g + jumpPoint->h;
      m_openList.Add(jumpPoint);
    }
  }
}

TArray<FJPSCell*> AJPSPathfinderActor::PruneStraight(FJPSCell* a_current, FVector a_jumpDirection)
{
  TArray<FJPSCell*> neighbors;

  //make sure we can move to the node
  FJPSCell* c = m_grid->GetCellByIndex(a_current->gridIndex + a_jumpDirection);
  if (!c || !c->walkable)
    return neighbors;
  //add the node we're trying to move to
  c->parentCell = a_current;
  neighbors.Add(c);

  //does this cell have a forced neighbor
  bool forced = false;

  //set the correct offsets based on the direction we're trying to move in
  FVector offset[2];
  if (a_jumpDirection.X != 0) //moving horizontally
  {
    offset[0] = FVector(0, 1, 0);
    offset[1] = FVector(0, -1, 0);
  }
  else //moving vertically
  {
    offset[0] = FVector(1, 0, 0);
    offset[1] = FVector(-1, 0, 0);
  }

  //now check these offets on forced neighbors
  for (int i = 0; i < 2; i++)
  {
    if (CheckStraightForcedNeighbor(a_current, offset[i], a_jumpDirection))
    {
      //forced neighbor check makes sure the cell exists, no need for null checking
      FJPSCell* forced = m_grid->GetCellByIndex(a_current->gridIndex + a_jumpDirection + offset[i]);
      forced->parentCell = a_current;
      neighbors.Add(forced); //if true, the neighbor is forced and we add it to the list of neighbors to check
    }
  }

  return neighbors;
}

TArray<FJPSCell*> AJPSPathfinderActor::PruneDiagonal(FJPSCell* a_current, FVector a_jumpDirection)
{
  TArray<FJPSCell*> neighbors;
  //check if we can even move diagonal (if c1 && c2 !walkable, it means we cannot move diagonal , movement would be blocked)
  FJPSCell* c1 = m_grid->GetCellByIndex(a_current->gridIndex + FVector(a_jumpDirection.X, 0, 0));
  FJPSCell* c2 = m_grid->GetCellByIndex(a_current->gridIndex + FVector(0, a_jumpDirection.Y, 0));

  //check if straight cells in the diagonal direction (dir.x and dir.y) are available, if atleast one of them is, we can move diagonal
  if (c1 && c1->walkable)
    c1->parentCell = a_current, neighbors.Add(c1);
  if (c2 && c2->walkable)
    c2->parentCell = a_current, neighbors.Add(c2);

  //only add the diagonal if atleast one of the other neighbors is added, otherwise they would block the diagonal movement
  if (neighbors.Num() > 0) 
  {
    FJPSCell* c = m_grid->GetCellByIndex(a_current->gridIndex + a_jumpDirection);
    if (c && c->walkable)
    {
      c->parentCell = a_current;
      neighbors.Add(c);
    }
  }

  //now check for forced neighbors
  FVector offset(-a_jumpDirection.X, 0, 0);
  if (CheckDiagonalForcedNeighbor(a_current, offset, a_jumpDirection))
  {
    FJPSCell* c = m_grid->GetCellByIndex(a_current->gridIndex + FVector(0, a_jumpDirection.Y, 0) + offset);
    c->parentCell = a_current;
    neighbors.Add(c);
  }

  offset = FVector(0, -a_jumpDirection.Y, 0);
  if (CheckDiagonalForcedNeighbor(a_current, offset, a_jumpDirection))
  {
    FJPSCell* c = m_grid->GetCellByIndex(a_current->gridIndex + FVector(a_jumpDirection.X, 0, 0) + offset);
    c->parentCell = a_current;
    neighbors.Add(c);
  }

  return neighbors;
}

short AJPSPathfinderActor::GetGValueJPS(FJPSCell* a_jumpPoint, FJPSCell* a_parent)
{
  FVector dir = a_jumpPoint->gridIndex - a_parent->gridIndex;
  if (dir.X != 0 && dir.Y != 0)
    return FMath::Abs(((a_jumpPoint->gridIndex.X - a_parent->gridIndex.X)) * 14);
  else
  {
    if (dir.X != 0)
      return FMath::Abs(((a_jumpPoint->gridIndex.X - a_parent->gridIndex.X)) * 10);
    else
      return FMath::Abs(((a_jumpPoint->gridIndex.Y - a_parent->gridIndex.Y)) * 10);
  }
}

FJPSCell* AJPSPathfinderActor::FindJumpPoint(FJPSCell* a_cell, FVector a_dir, FJPSCell* a_startCell, FJPSCell* a_targetCell)
{
  //start of by "jumping" one step in the desired direction
  FJPSCell* n = m_grid->GetCellByIndex(a_cell->gridIndex + a_dir);
  if (!n || !n->walkable) //stop jumping if we hit a block, and return null. This jump is invalid and we don't need to look at it any further
    return nullptr;
  if (n == a_targetCell) //we've reached our target, stop jumping and return n
    return n;
  //check if there is any forced neighbors on the node, if so, return n
  if (a_dir.X == 0 || a_dir.Y == 0) //straight move
  {
    FVector offset[2];
    if (a_dir.X != 0) //moving horizontally
      offset[0] = FVector(0, 1, 0), offset[1] = FVector(0, -1, 0);
    else //moving vertically
      offset[0] = FVector(1, 0, 0), offset[1] = FVector(-1, 0, 0);
    for (int i = 0; i < 2; i++)
    {
      if (CheckStraightForcedNeighbor(n, offset[i], a_dir))
        return n; //if a forced neighbor is found, return n so it can be furter examined in the next iteration
    }
  }
  else //diagonal move
  {
    //first check if a diagonal forced neighbor is found, if so, return n
    if (CheckDiagonalForcedNeighbor(n, FVector(-a_dir.X, 0, 0), a_dir))
      return n;
    if (CheckDiagonalForcedNeighbor(n, FVector(0, -a_dir.Y, 0), a_dir))
      return n;

    //if not returned at this point, check both straights of the diagonal for a jump point
    //if we find one, return n and end the jump
    if (FindJumpPoint(n, FVector(a_dir.X, 0, 0), a_startCell, a_targetCell))
      return n;
    if (FindJumpPoint(n, FVector(0, a_dir.Y, 0), a_startCell, a_targetCell))
      return n;
  }
  //keep recursing until a return is hit
  return FindJumpPoint(n, a_dir, a_startCell, a_targetCell);
}

bool AJPSPathfinderActor::CheckStraightForcedNeighbor(FJPSCell* a_cell, FVector a_offset, FVector a_dir)
{
  FJPSCell* c = m_grid->GetCellByIndex(a_cell->gridIndex + a_offset);
  if (!c || !c->walkable) //test if the node diagonal from our starting node is avaiable
  {
    c = m_grid->GetCellByIndex(a_cell->gridIndex + a_dir + a_offset);
    if (c && c->walkable) //if it is->
      return true; //c is a forced neighbor, return true
  }
  return false;
}

bool AJPSPathfinderActor::CheckDiagonalForcedNeighbor(FJPSCell* a_cell, FVector a_offset, FVector a_dir)
{
  FJPSCell* c = m_grid->GetCellByIndex(a_cell->gridIndex + a_offset);
  if (!c || !c->walkable) //test if the node diagonal from our starting node is avaiable
  {
    FVector offset = (a_offset.X == 0) ? FVector(a_dir.X, 0, 0) : FVector(0, a_dir.Y, 0);
    c = m_grid->GetCellByIndex(a_cell->gridIndex + offset + a_offset);
    if (c && c->walkable) //if it is->
      return true; //c is forced, return true
  }
  return false;
}

int AJPSPathfinderActor::GetHeuristic(FJPSCell* a_cell)
{
  FJPSPathfindingPacket currentPacket;
  m_pathQueue.Peek(currentPacket);

  int x = (int)(FMath::Abs(a_cell->gridIndex.X - currentPacket.targetCell->gridIndex.X));
  int y = (int)(FMath::Abs(a_cell->gridIndex.Y - currentPacket.targetCell->gridIndex.Y));

  return (x + y) * 10;
}


void AJPSPathfinderActor::Clear()
{
  //Reset all used cells
  for (FJPSCell* cell : m_closedList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_closedList.Empty();

  for (FJPSCell* cell : m_openList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_openList.Empty();
}

void AJPSPathfinderActor::DrawDebug()
{
  if (!m_updateStepBased)
    return;

  FJPSPathfindingPacket current;
  if (m_pathQueue.Peek(current))
  {
    DrawDebugPoint(
      GetWorld(),
      current.targetCell->position,
      15,
      FColor::Yellow
    ); 
    DrawDebugPoint(
      GetWorld(),
      current.startCell->position,
      15,
      FColor::Green
    );
  }

  for (FJPSCell* c : m_openList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Blue
    );

    FJPSCell* parent = c;
    if (parent->parentCell)
    {
      DrawDebugPoint(
        GetWorld(),
        parent->parentCell->position,
        15,
        FColor::Blue
      );

      DrawDebugLine(GetWorld(),
        parent->position, parent->parentCell->position,
        FColor::Blue, false, -1, 0, 5.f
      );
    }
  }

  for (FJPSCell* c : m_closedList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Red
    );

    FJPSCell* parent = c;
    if (parent->parentCell)
    {
      DrawDebugPoint(
        GetWorld(),
        parent->parentCell->position,
        15,
        FColor::Red
      );

      DrawDebugLine(GetWorld(),
        parent->position, parent->parentCell->position,
        FColor::Red, false, -1, 0, 5.f
      );
    }
  }
}

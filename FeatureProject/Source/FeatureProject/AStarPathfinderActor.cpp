//Fill out your copyright notice in the Description page of Project Settings.

#include "FeatureProject.h"
#include "AStarAgentComponent.h"
#include "AStarPathfinderActor.h"


//Sets default values
AAStarPathfinderActor::AAStarPathfinderActor()
{
 	//Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

//Called when the game starts or when spawned
void AAStarPathfinderActor::BeginPlay()
{
	Super::BeginPlay();

  for (TActorIterator<AAStarGrid> Itr(GetWorld()); Itr; ++Itr)
    m_grid = *Itr;

  if (!m_grid)
    UE_LOG(LogTemp, Error, TEXT("ERROR: could not find grid, AStartPathfinderActor.cpp"));

}

void AAStarPathfinderActor::RequestPath(UActorComponent* a_requester, FAStarCell* a_startCell, FAStarCell* a_targetCell)
{
  FAStarPathfindingPacket newPacket;
  newPacket.requestingActor = a_requester;
  newPacket.startCell = a_startCell;
  newPacket.targetCell = a_targetCell;

  m_pathQueue.Enqueue(newPacket);
}

//Called every frame
void AAStarPathfinderActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

  if (!m_pathQueue.IsEmpty()) //the queue is not empty, there is an actor requesting a path
  {
    FAStarPathfindingPacket currentPacket;
    m_pathQueue.Peek(currentPacket);
    currentPacket.startCell->parentCell = nullptr;
    m_openList.Add(currentPacket.startCell);

    bool targetFound = false;
    while (!targetFound)
    {
      if (m_openList.Num() == 0)
      {
        //no valid path can be found
        m_pathQueue.Dequeue(currentPacket); //remove the requester from the queue since path is invalid
        ((UAStarAgentComponent*)(currentPacket.requestingActor))->PathRequestFailed();
        return;
      }

      FAStarCell* cell = m_openList[0];

      //get the node with the lowest f value of the open list
      for (FAStarCell* c : m_openList)
      {
        if (cell->f > c->f)
          cell = c;
      }

      //remove lowest f node from open list and add to closed list
      m_openList.Remove(cell);

      if (cell != currentPacket.targetCell)
      {
        AddNeighbors(cell);

        m_closedList.Add(cell);
        cell->cellType = EListStatus::ON_CLOSED;
      }
      else
        targetFound = true;
    }

    //create the found path and set it on the requesting agent
    FPath* path = new FPath();
    CreatePath(currentPacket.targetCell, path);
    ((UAStarAgentComponent*)(currentPacket.requestingActor))->SetPath(path);
    m_pathQueue.Dequeue(currentPacket);
    if (m_autoClear)
      Clear();
  }

  //used for debugging, if we don't auto clear we can see which cells have been added to the open and closed lists
  if (!m_autoClear && m_clear)
  {
    Clear();
    m_clear = false;
  }

  if (m_debugdraw)
    DrawDebug();
}

void AAStarPathfinderActor::CreatePath(FAStarCell* a_cell, FPath* a_path)
{
  //backtrace the path to the starting node
  a_path->path.Add(a_cell->position);
  if (a_cell->parentCell)
  {
    CreatePath(a_cell->parentCell, a_path);
  }
}

void AAStarPathfinderActor::AddNeighbors(FAStarCell* a_cell)
{
  for (int i = 0; i < 8; i++)
  {
    short g = (i < 4) ? 10 : 14; //we set straight neighbors first, so if i < 4 it means we're dealing with a straight neighbor
    if (a_cell->neighbors[i])
    {
      FAStarCell* cell = a_cell->neighbors[i];
      if (cell->cellType != EListStatus::ON_CLOSED)
      {
        if (cell->cellType != EListStatus::ON_OPEN)
        {
          cell->g = a_cell->g + g;
          cell->cellType = EListStatus::ON_OPEN;
          cell->parentCell = a_cell;
          cell->h = GetHeuristic(cell);
          cell->f = cell->g + cell->h;
          m_openList.Add(cell);
        }
        else
        {
          //check if the path to the current node is shorter than the one we have
          Relax(cell, a_cell, g);
        }
      }
    }
  }
}

void AAStarPathfinderActor::Relax(FAStarCell* a_targetCell, FAStarCell* a_startCell, int a_cost)
{
  FAStarCell* startCell = a_startCell;
  FAStarCell* targetCell = a_targetCell;

  //check if path to targetnode is shorter if we go through startnode instead of current path
  if (targetCell->g > startCell->g + a_cost)
  {
    targetCell->parentCell = startCell;
    targetCell->g = startCell->g + a_cost;
    targetCell->h = GetHeuristic(targetCell);
    targetCell->f = targetCell->g + targetCell->h;
  }
}

int AAStarPathfinderActor::GetHeuristic(FAStarCell* a_cell)
{
  FAStarPathfindingPacket currentPacket;
  m_pathQueue.Peek(currentPacket);
  //use manhattan hueristic to calculate h value
  int x = (int)(FMath::Abs(a_cell->position.X - currentPacket.targetCell->position.X) / 100.0f);
  int y = (int)(FMath::Abs(a_cell->position.Y - currentPacket.targetCell->position.Y) / 100.0f);

  return (x + y) * 10;
}

void AAStarPathfinderActor::Clear()
{
  //Reset all used cells
  for (FAStarCell* cell : m_closedList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_closedList.Empty();

  for (FAStarCell* cell : m_openList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_openList.Empty();
}

void AAStarPathfinderActor::DrawDebug()
{
  FAStarPathfindingPacket current;
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

  for (FAStarCell* c : m_openList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Blue
    );

    FAStarCell* parent = c;
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

  for (FAStarCell* c : m_closedList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Red
    );

    FAStarCell* parent = c;
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
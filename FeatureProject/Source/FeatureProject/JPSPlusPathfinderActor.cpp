// Fill out your copyright notice in the Description page of Project Settings.
// Created by Eric Troost, 132385
// references:
//http://gdcvault.com/play/1022094/JPS-Over-100x-Faster-than
//http://zerowidth.com/2013/05/05/jump-point-search-explained.html
//http://grastien.net/ban/articles/hg-aaai11.pdf

/*
* Copyright(c) 2014 - 2015, Steve Rabin
* All rights reserved.
* Copyright (c) 2014-2015, Steve Rabin
* All rights reserved.
*
* An explanation of the JPS+ algorithm is contained in Chapter 14
* of the book Game AI Pro 2, edited by Steve Rabin, CRC Press, 2015.
* A presentation on Goal Bounding titled "JPS+: Over 100x Faster than A*"
* can be found at www.gdcvault.com from the 2015 GDC AI Summit.
* A copy of this code is on the website http://www.gameaipro.com.
*
* If you develop a way to improve this code or make it faster, please
* contact steve.rabin@gmail.com and share your insights. I would
* be equally eager to hear from anyone integrating this code or using
* the Goal Bounding concept in a commercial application or game.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * The name of the author may not be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY STEVE RABIN ``AS IS'' AND ANY
* EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL <copyright holder> BE LIABLE FOR ANY
* DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
* (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
* LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "FeatureProject.h"
#include "JPSPlusAgentComponent.h"
#include "JPSPlusPathfinderActor.h"

// Sets default values
AJPSPlusPathfinderActor::AJPSPlusPathfinderActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AJPSPlusPathfinderActor::BeginPlay()
{
	Super::BeginPlay();

  for (TActorIterator<AJPSPlusGridActor> Itr(GetWorld()); Itr; ++Itr)
    m_grid = *Itr;

  if (!m_grid)
    UE_LOG(LogTemp, Error, TEXT("ERROR: could not find grid"));

  //Bind all jump functions
  m_jumpMap.Add(EJumpDirections::UP_LEFT, &AJPSPlusPathfinderActor::JumpUpLeft);
  m_jumpMap.Add(EJumpDirections::UP, &AJPSPlusPathfinderActor::JumpUp);
  m_jumpMap.Add(EJumpDirections::UP_RIGHT, &AJPSPlusPathfinderActor::JumpUpRight);
  m_jumpMap.Add(EJumpDirections::LEFT, &AJPSPlusPathfinderActor::JumpLeft);
  m_jumpMap.Add(EJumpDirections::RIGHT, &AJPSPlusPathfinderActor::JumpRight);
  m_jumpMap.Add(EJumpDirections::DOWN_LEFT, &AJPSPlusPathfinderActor::JumpDownLeft);
  m_jumpMap.Add(EJumpDirections::DOWN, &AJPSPlusPathfinderActor::JumpDown);
  m_jumpMap.Add(EJumpDirections::DOWN_RIGHT, &AJPSPlusPathfinderActor::JumpDownRight);

  CreateDirectionJumpMap();
  CreateDirectionDistanceMap();
  CreateJumpDirectionMap();
}

void AJPSPlusPathfinderActor::CreateDirectionJumpMap()
{
  //up left
  TArray<EJumpDirections> jumpDirs;
  jumpDirs.Add(EJumpDirections::UP_LEFT), jumpDirs.Add(EJumpDirections::UP), jumpDirs.Add(EJumpDirections::LEFT);
  m_directionJumpMap.Add(FVector(-1, -1, 0), jumpDirs);
  //up
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::LEFT), jumpDirs.Add(EJumpDirections::UP_LEFT), jumpDirs.Add(EJumpDirections::UP), jumpDirs.Add(EJumpDirections::UP_RIGHT), jumpDirs.Add(EJumpDirections::RIGHT);
  m_directionJumpMap.Add(FVector(0, -1, 0), jumpDirs);
  //up right
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::UP_RIGHT), jumpDirs.Add(EJumpDirections::UP), jumpDirs.Add(EJumpDirections::RIGHT);
  m_directionJumpMap.Add(FVector(1, -1, 0), jumpDirs);
  //left
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::DOWN), jumpDirs.Add(EJumpDirections::DOWN_LEFT), jumpDirs.Add(EJumpDirections::LEFT), jumpDirs.Add(EJumpDirections::UP_LEFT), jumpDirs.Add(EJumpDirections::UP);
  m_directionJumpMap.Add(FVector(-1, 0, 0), jumpDirs);
  //right
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::UP), jumpDirs.Add(EJumpDirections::UP_RIGHT), jumpDirs.Add(EJumpDirections::RIGHT), jumpDirs.Add(EJumpDirections::DOWN_RIGHT), jumpDirs.Add(EJumpDirections::DOWN);
  m_directionJumpMap.Add(FVector(1, 0, 0), jumpDirs);
  //down left
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::DOWN_LEFT), jumpDirs.Add(EJumpDirections::DOWN), jumpDirs.Add(EJumpDirections::LEFT);
  m_directionJumpMap.Add(FVector(-1, 1, 0), jumpDirs);
  //down
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::LEFT), jumpDirs.Add(EJumpDirections::DOWN_LEFT), jumpDirs.Add(EJumpDirections::DOWN), jumpDirs.Add(EJumpDirections::DOWN_RIGHT), jumpDirs.Add(EJumpDirections::RIGHT);
  m_directionJumpMap.Add(FVector(0, 1, 0), jumpDirs);
  //down right
  jumpDirs.Empty();
  jumpDirs.Add(EJumpDirections::DOWN_RIGHT), jumpDirs.Add(EJumpDirections::DOWN), jumpDirs.Add(EJumpDirections::RIGHT);
  m_directionJumpMap.Add(FVector(1, 1, 0), jumpDirs);
}

void AJPSPlusPathfinderActor::CreateDirectionDistanceMap()
{
  m_directionDistanceMap.Add(FVector(-1, -1, 0), 0);
  m_directionDistanceMap.Add(FVector(0, -1, 0), 1);
  m_directionDistanceMap.Add(FVector(1, -1, 0), 2);
  m_directionDistanceMap.Add(FVector(-1, 0, 0), 3);
  m_directionDistanceMap.Add(FVector(1, 0, 0), 4);
  m_directionDistanceMap.Add(FVector(-1, 1, 0), 5);
  m_directionDistanceMap.Add(FVector(0, 1, 0), 6);
  m_directionDistanceMap.Add(FVector(1, 1, 0), 7);
}

void AJPSPlusPathfinderActor::CreateJumpDirectionMap()
{
  m_jumpDirectionMap.Add(EJumpDirections::UP_LEFT, FVector(-1, -1, 0));
  m_jumpDirectionMap.Add(EJumpDirections::UP, FVector(0, -1, 0));
  m_jumpDirectionMap.Add(EJumpDirections::UP_RIGHT, FVector(1, -1, 0));
  m_jumpDirectionMap.Add(EJumpDirections::LEFT, FVector(-1, 0, 0));
  m_jumpDirectionMap.Add(EJumpDirections::RIGHT, FVector(1, 0, 0));
  m_jumpDirectionMap.Add(EJumpDirections::DOWN_LEFT, FVector(-1, 1, 0));
  m_jumpDirectionMap.Add(EJumpDirections::DOWN, FVector(0, 1, 0));
  m_jumpDirectionMap.Add(EJumpDirections::DOWN_RIGHT, FVector(1, 1, 0));
}

void AJPSPlusPathfinderActor::RequestPath(UActorComponent* a_requester, FPathfindingCell* a_startCell, FPathfindingCell* a_targetCell)
{
  FJPSPlusPathfindingPacket newPacket;
  newPacket.requestingActor = a_requester;
  newPacket.startCell = a_startCell;
  newPacket.targetCell = a_targetCell;

  m_pathQueue.Enqueue(newPacket);
}

// Called every frame
void AJPSPlusPathfinderActor::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
  DebugDraw();

  if (!m_pathQueue.IsEmpty()) // the queue is not empty, there is an actor requesting a path
  {
    FJPSPlusPathfindingPacket currentPacket;
    m_pathQueue.Peek(currentPacket);

    AddStartCell(currentPacket);

    bool res = SearchLoop(currentPacket);

    if (m_updateStepBased && !res)
      return;

    if (res)
    {
      FPath* path = new FPath();
      CreatePath(currentPacket.targetCell, path);
      ((UJPSPlusAgentComponent*)(currentPacket.requestingActor))->SetPath(path);
      m_pathQueue.Dequeue(currentPacket);
      m_isFindingPath = false;
      Clear();
    }
  }
}

void AJPSPlusPathfinderActor::AddStartCell(FJPSPlusPathfindingPacket &a_packet)
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

bool AJPSPlusPathfinderActor::SearchLoop(FJPSPlusPathfindingPacket &a_packet)
{
  bool targetFound = false;
  while (!targetFound)
  {
    if (m_updateStepBased && !m_debugStep)
      break;

    if ((m_openList.Num() == 0 && m_fastStack.Num() == 0) || m_openList.Num() > 1000) // It should never be over 1000, something went wrong, so terminate the path if it is
    {
      // no valid path can be found
      // remove the requester from the queue since path is invalid
      m_pathQueue.Dequeue(a_packet);
      // Tell the requester their path request failed
      ((UJPSPlusAgentComponent*)(a_packet.requestingActor))->PathRequestFailed();
      m_isFindingPath = false;
      Clear();
      return false;
    }

    FPathfindingCell* cell = m_openList[0];
    //if (m_fastStack.Num())
    //  cell = m_fastStack.Pop();
    //else
    //  cell = m_openList.Pop();
    for (auto c : m_openList)
    {
      if (c->f < cell->f)
        cell = c;
    }
    m_openList.Remove(cell);

    if (cell != a_packet.targetCell)
    {
      AddNeighbours(cell);

      m_closedList.Add(cell);
      cell->cellType = EListStatus::ON_CLOSED;
    }
    else
      targetFound = true;

    m_debugStep = false;
  }

  return targetFound;
}

int AJPSPlusPathfinderActor::GetHeuristic(FPathfindingCell* a_cell)
{
  FJPSPlusPathfindingPacket currentPacket;
  m_pathQueue.Peek(currentPacket);

  int x = (int)(FMath::Abs(a_cell->position.X - currentPacket.targetCell->position.X) / 100.0f);
  int y = (int)(FMath::Abs(a_cell->position.Y - currentPacket.targetCell->position.Y) / 100.0f);

  return (x + y) * 10;
}

void AJPSPlusPathfinderActor::CreatePath(FPathfindingCell* a_cell, FPath* a_path)
{
  // backtrace the path to the starting node
  a_path->path.Add(a_cell->position);
  if (a_cell->parentCell)
  {
    CreatePath(a_cell->parentCell, a_path);
  }
}

void AJPSPlusPathfinderActor::Clear()
{
  // Reset all used cells
  for (FPathfindingCell* cell : m_closedList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_closedList.Empty();

  for (FPathfindingCell* cell : m_openList)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_openList.Empty();

  for (FPathfindingCell* cell : m_fastStack)
  {
    cell->f = cell->h = cell->g = 0;
    cell->cellType = EListStatus::NONE;
    cell->parentCell = nullptr;
  }
  m_fastStack.Empty();
}

//####################################################
// JUMP POINT SEARCH NEIGHBOUR FINDING
//####################################################

void AJPSPlusPathfinderActor::AddNeighbours(FPathfindingCell* a_cell)
{
  // start of by pruning neighbours we don't want to look at
  if (a_cell->parentCell)
  {
    // Get our movement direction
    FVector direction = a_cell->position - a_cell->parentCell->position;
    direction.X = FMath::Clamp<int>(direction.X, -1, 1);
    direction.Y = FMath::Clamp<int>(direction.Y, -1, 1);

    TArray<EJumpDirections> jumps = m_directionJumpMap[direction];
    for (EJumpDirections jump : jumps)
    {
      short distance = a_cell->distanceToJumppoint[m_directionDistanceMap[m_jumpDirectionMap[jump]]];
      (this->*m_jumpMap[jump])(a_cell, distance);
    }
  }
  else // no parent -> this is our starting node. Try all neighbours
  {
    (this->*m_jumpMap[EJumpDirections::UP_LEFT])    (a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(-1, -1, 0)]]);
    (this->*m_jumpMap[EJumpDirections::UP])					(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(0, -1, 0)]]);
    (this->*m_jumpMap[EJumpDirections::UP_RIGHT])		(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(1, -1, 0)]]);
    (this->*m_jumpMap[EJumpDirections::LEFT])				(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(-1, 0, 0)]]);
    (this->*m_jumpMap[EJumpDirections::RIGHT])			(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(1, 0, 0)]]);
    (this->*m_jumpMap[EJumpDirections::DOWN_LEFT])	(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(-1, 1, 0)]]);
    (this->*m_jumpMap[EJumpDirections::DOWN])				(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(0, 1, 0)]]);
    (this->*m_jumpMap[EJumpDirections::DOWN_RIGHT])	(a_cell, a_cell->distanceToJumppoint[m_directionDistanceMap[FVector(1, 1, 0)]]);
  }
}

//########################################################################
//########################################################################
//Jump functions
//########################################################################
//########################################################################
void AJPSPlusPathfinderActor::JumpUpLeft(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  //check for target jump point
  if (currentIndex.X > goalIndex.X && currentIndex.Y > goalIndex.Y) //our goal node is in the direction we're jumping towards
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    FVector diff = currentIndex - goalIndex;
    short smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;

    if (smallestDiff <= absDistance)
    {
      short g = a_current->g + (FMath::Abs(smallestDiff) * 14);
      FVector newIndex = FVector(currentIndex.X - smallestDiff, currentIndex.Y - smallestDiff, currentIndex.Z);
      FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
      AddCellToOpenList(newCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    short g = a_current->g + (a_jumpDistance * 14);
    FVector newIndex = currentIndex + FVector(-a_jumpDistance, -a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpUp(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // start of by checking for a target jump point
  if (currentIndex.X == goalIndex.X && currentIndex.Y > goalIndex.Y) // on the same X, and bigger Y -> there might be a target node that can help us reach the goal
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    if ((currentIndex.Y - absDistance) <= goalIndex.Y) // the goal is closer than our target jump point, we can jump straight to the goal
    {
      short diff = currentIndex.Y - goalIndex.Y;
      short g = a_current->g + (FMath::Abs(diff) * 10);
      AddCellToOpenList(m_goalCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0) // not dealing with walls
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(0, -a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 10);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpUpRight(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // check for target jump point
  if (currentIndex.X < goalIndex.X && currentIndex.Y > goalIndex.Y) // our goal node is in the direction we're jumping towards
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    FVector diff = FVector(goalIndex.X - currentIndex.X, currentIndex.Y - goalIndex.Y, currentIndex.Z);
    short smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;

    if (smallestDiff <= absDistance)
    {
      short g = a_current->g + (FMath::Abs(smallestDiff) * 14);
      FVector newIndex = FVector(currentIndex.X + smallestDiff, currentIndex.Y - smallestDiff, currentIndex.Z);
      FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
      AddCellToOpenList(newCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    short g = a_current->g + (a_jumpDistance * 14);
    FVector newIndex = currentIndex + FVector(a_jumpDistance, -a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpLeft(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // start of by checking for a target jump point
  if (currentIndex.Y == goalIndex.Y && currentIndex.X > goalIndex.X)
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    if ((currentIndex.X - absDistance) <= goalIndex.X) // we can jump straight to the goal
    {
      short diff = currentIndex.X - goalIndex.X;
      short g = a_current->g + (FMath::Abs(diff) * 10);
      AddCellToOpenList(m_goalCell, a_current, g);
      return;
    }
  }

  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(-a_jumpDistance, 0, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 10);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpRight(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // start of by checking for a target jump point
  if (currentIndex.Y == goalIndex.Y && currentIndex.X < goalIndex.X)
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    if ((currentIndex.X + absDistance) >= goalIndex.X) // we can jump straight to the goal
    {
      short diff = currentIndex.X - goalIndex.X;
      short g = a_current->g + (FMath::Abs(diff) * 10);
      AddCellToOpenList(m_goalCell, a_current, g);
      return;
    }
  }

  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(a_jumpDistance, 0, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 10);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpDownLeft(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // check for target jump point
  if (currentIndex.X > goalIndex.X && currentIndex.Y < goalIndex.Y) // our goal node is in the direction we're jumping towards
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    FVector diff = FVector(currentIndex.X - goalIndex.X, goalIndex.Y - currentIndex.Y, currentIndex.Z);
    short smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;

    if (smallestDiff <= absDistance)
    {
      short g = a_current->g + (FMath::Abs(smallestDiff) * 14);
      FVector newIndex = FVector(currentIndex.X - smallestDiff, currentIndex.Y + smallestDiff, currentIndex.Z);
      FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
      AddCellToOpenList(newCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(-a_jumpDistance, a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 14);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpDown(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // start of by checking for a target jump point
  if (currentIndex.X == goalIndex.X && currentIndex.Y < goalIndex.Y) // on the same X, and smaller Y -> there might be a target node that can help us reach the goal
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    if ((currentIndex.Y + absDistance) >= goalIndex.Y) // the goal is closer than our target jump point, we can jump straight to the goal
    {
      short diff = goalIndex.Y - currentIndex.Y;
      short g = a_current->g + (FMath::Abs(diff) * 10);
      AddCellToOpenList(m_goalCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0) // not dealing with walls
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(0, a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 10);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}

void AJPSPlusPathfinderActor::JumpDownRight(FPathfindingCell* a_current, short a_jumpDistance)
{
  FVector currentIndex = a_current->gridIndex;
  FVector goalIndex = m_goalCell->gridIndex;

  // check for target jump point
  if (currentIndex.X < goalIndex.X && currentIndex.Y < goalIndex.Y) // our goal node is in the direction we're jumping towards
  {
    short absDistance = FMath::Abs(a_jumpDistance);

    FVector diff = goalIndex - currentIndex;
    short smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;

    if (smallestDiff <= absDistance)
    {
      short g = a_current->g + (FMath::Abs(smallestDiff) * 14);
      FVector newIndex = FVector(currentIndex.X + smallestDiff, currentIndex.Y + smallestDiff, currentIndex.Z);
      FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
      AddCellToOpenList(newCell, a_current, g);
      return;
    }
  }

  // if we're not dealing with a target jump point, test for potential jump
  if (a_jumpDistance > 0)
  {
    // we know we can immediatly jump to this distance (precalculated)
    FVector newIndex = currentIndex + FVector(a_jumpDistance, a_jumpDistance, 0);
    FPathfindingCell* newCell = m_grid->GetCellByIndex(newIndex);
    short g = a_current->g + (a_jumpDistance * 14);
    AddCellToOpenList(newCell, a_current, g);
    return;
  }
}
//########################################################################
//########################################################################
//########################################################################
//########################################################################

void AJPSPlusPathfinderActor::AddCellToOpenList(FPathfindingCell* a_newCell, FPathfindingCell* a_parentCell, short a_g)
{
  if (a_newCell->cellType == EListStatus::NONE)
  {
    // if we find a jump point, give it all its values and add it to the open list, as with Astar
    a_newCell->g = a_g;
    a_newCell->cellType = EListStatus::ON_OPEN;
    a_newCell->parentCell = a_parentCell;
    a_newCell->h = GetHeuristic(a_newCell);
    a_newCell->f = a_newCell->g + a_newCell->h;

    // TODO - fix fast stack
    //if (a_newCell->f >= a_parentCell->f)
    //  m_fastStack.Add(a_newCell);
    //else
    m_openList.Add(a_newCell);
  }
  else if (a_newCell->cellType == EListStatus::ON_OPEN
    && a_g < a_newCell->g) // this is a cheaper path to the current node, so let's use it (relax)
  {
    // no need to recalculate h-cost, since it was previously calculated
    short h = a_newCell->f - a_newCell->g;
    a_newCell->parentCell = a_parentCell;
    a_newCell->g = a_g;
    a_newCell->f = a_newCell->g + h;
  }
}

void AJPSPlusPathfinderActor::DebugDraw()
{
  if (!m_updateStepBased)
    return;

  FJPSPlusPathfindingPacket current;
  if (m_pathQueue.Peek(current))
  {
    DrawDebugPoint(
      GetWorld(),
      current.targetCell->position,
      15,
      FColor::Yellow
      );
  }

  for (FPathfindingCell* c : m_openList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Blue
      );

    FPathfindingCell* parent = c;
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

  for (FPathfindingCell* c : m_closedList)
  {
    DrawDebugPoint(
      GetWorld(),
      c->position,
      15,
      FColor::Red
      );

    FPathfindingCell* parent = c;
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

  // draw info for target jump points
  if (!m_pathQueue.IsEmpty())
  {
    FJPSPlusPathfindingPacket currentPacket;
    m_pathQueue.Peek(currentPacket);

    FPathfindingCell* target = currentPacket.targetCell;

    // draw x-line
    FPathfindingCell* startCell = m_grid->GetCellByIndex(FVector(target->gridIndex.X, 0, 0));
    FPathfindingCell* endCell = m_grid->GetCellByIndex(FVector(target->gridIndex.X, m_grid->GetGridSize().Y - 1, 0));
    if (startCell && endCell)
    {
      DrawDebugLine(GetWorld(),
        startCell->position, endCell->position,
        FColor::Yellow, false, -1, 0, 5.f
        );
    }

    // draw y-line
    startCell = m_grid->GetCellByIndex(FVector(0, target->gridIndex.Y, 0));
    endCell = m_grid->GetCellByIndex(FVector(m_grid->GetGridSize().X - 1, target->gridIndex.Y, 0));
    if (startCell && endCell)
    {
      DrawDebugLine(GetWorld(),
        startCell->position, endCell->position,
        FColor::Yellow, false, -1, 0, 5.f
        );
    }

    // draw diagonal lines
    // get top left corner
    FVector index = target->gridIndex;
    FVector goal = FVector::ZeroVector;
    FVector diff = index - goal;
    short smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;
    FVector newIndex = FVector(index.X - smallestDiff, index.Y - smallestDiff, index.Z);
    startCell = m_grid->GetCellByIndex(newIndex);
    // bottom right corner
    index = target->gridIndex;
    goal = m_grid->GetGridSize() - FVector(1, 1, 0);
    diff = goal - index;
    smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;
    newIndex = FVector(index.X + smallestDiff, index.Y + smallestDiff, index.Z);
    endCell = m_grid->GetCellByIndex(newIndex);
    if (startCell && endCell)
    {
      DrawDebugLine(GetWorld(),
        startCell->position, endCell->position,
        FColor::Yellow, false, -1, 0, 5.f
        );
    }

    // get top right corner
    index = target->gridIndex;
    goal = FVector(m_grid->GetGridSize().X - 1, 0, 0);
    diff = FVector(goal.X - index.X, index.Y - goal.Y, index.Z);
    smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;
    newIndex = FVector(index.X + smallestDiff, index.Y - smallestDiff, index.Z);
    startCell = m_grid->GetCellByIndex(newIndex);
    //bottom left corner
    index = target->gridIndex;
    goal = FVector(0, m_grid->GetGridSize().Y - 1, 0);
    diff = FVector(index.X - goal.X, goal.Y - index.Y, index.Z);;
    smallestDiff = (diff.X < diff.Y) ? diff.X : diff.Y;
    newIndex = FVector(index.X - smallestDiff, index.Y + smallestDiff, index.Z);
    endCell = m_grid->GetCellByIndex(newIndex);
    if (startCell && endCell)
    {
      DrawDebugLine(GetWorld(),
        startCell->position, endCell->position,
        FColor::Yellow, false, -1, 0, 5.f
        );
    }
  }
}
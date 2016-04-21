//Fill out your copyright notice in the Description page of Project Settings.

#include "FeatureProject.h"
#include "JPSPlusAgentComponent.h"
#include "AgentPathFollowingComponent.h"

//Sets default values for this component's properties
UJPSPlusAgentComponent::UJPSPlusAgentComponent()
{
	//Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	//off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	//...
}


//Called when the game starts
void UJPSPlusAgentComponent::BeginPlay()
{
	Super::BeginPlay();

  for (TActorIterator<AJPSPlusGridActor> Itr(GetWorld()); Itr; ++Itr)
    m_grid = *Itr;

  for (TActorIterator<AJPSPlusPathfinderActor> Itr(GetWorld()); Itr; ++Itr)
    m_pathfinder = *Itr;

  if (!m_pathfinder)
    UE_LOG(LogTemp, Warning, TEXT("ERROR: could not find pathfinder, JPSPlusAgentComponent.cpp")); 
  if (!m_grid)
    UE_LOG(LogTemp, Warning, TEXT("ERROR: could not find grid, JPSPlusAgentComponent.cpp")); 
}


//Called every frame
void UJPSPlusAgentComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );

  if (m_calculatePointsAndRequestPath)
  {
    CalculatePointsAndRequestPath(m_targetPosDebug);
    m_calculatePointsAndRequestPath = false;
  }
  //make sure we still have path nodes left on our path, and draw debug info if needed
  if (m_path)
  {
    if (!m_path->path.Num())
    {
      delete m_path;
      m_path = nullptr;
    }
    else if (m_drawDebug)
      DrawDebugInformation();
  }
}

void UJPSPlusAgentComponent::CancelCurrentPath()
{
  if (m_path)
  {
    delete m_path;
    m_path = nullptr;
  }

  for (auto comp : GetOwner()->GetComponentsByClass(UAgentPathFollowingComponent::StaticClass()))
    ((UAgentPathFollowingComponent*)comp)->CancelPath();

  //make sure we're not on an invalid node (can happen during diagonal moves)
  FVector pos(GetOwner()->GetActorLocation());
  FPathfindingCell* current = m_grid->GetCellByPosition(pos);
  if (current && !current->walkable)
  {
    //push ourselves out
    FVector dir = pos - current->position;
    dir.X = FMath::Clamp<float>(dir.X, -1, 1);
    dir.Y = FMath::Clamp<float>(dir.Y, -1, 1);
    FPathfindingCell* target = m_grid->GetCellByIndex(current->gridIndex + dir);
    if (target)
      GetOwner()->SetActorLocation(target->position);
  }
}

bool UJPSPlusAgentComponent::IsTraversingPath()
{
  return (m_path != nullptr || m_waitingForPath);
}

void UJPSPlusAgentComponent::SetPath(FPath* a_path)
{
  m_path = a_path;
  m_waitingForPath = false;
}

FPath* UJPSPlusAgentComponent::GetPath()
{
  return m_path;
}

void UJPSPlusAgentComponent::PathRequestFailed()
{
  m_waitingForPath = false;
}

void UJPSPlusAgentComponent::CalculatePointsAndRequestPath(FVector a_targetLocation)
{
  FVector startPos = GetOwner()->GetActorLocation();
  m_startCell = m_grid->GetCellByPosition(startPos);
  m_targetCell = m_grid->GetCellByPosition(a_targetLocation);

  if (m_pathfinder && m_startCell && m_targetCell)
    m_pathfinder->RequestPath(this, m_startCell, m_targetCell);

  m_waitingForPath = true;
}

void UJPSPlusAgentComponent::DrawDebugInformation()
{
  if (m_path->path.Num() == 0)
    return;

  for (int i = 0; i < m_path->path.Num() - 1; i++)
  {
    FVector pos1 = m_path->path[i];
    FVector pos2 = m_path->path[i + 1];

    DrawDebugPoint(
      GetWorld(),
      FVector(pos1.X, pos1.Y, pos1.Z),
      7,
      FColor(255, 255, 0)
      );

    DrawDebugLine(GetWorld(),
      FVector(pos1.X, pos1.Y, pos1.Z), FVector(pos2.X, pos2.Y, pos2.Z),
      FColor(255, 255, 0), false, -1, 0, 5.f
      );
  }

  DrawDebugPoint(
    GetWorld(),
    FVector(m_path->path.Last().X, m_path->path.Last().Y, m_path->path.Last().Z),
    7,
    FColor(255, 255, 0)
    );
}

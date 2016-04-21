// Fill out your copyright notice in the Description page of Project Settings.

#include "FeatureProject.h"
#include "AgentPathFollowingComponent.h"


// Sets default values for this component's properties
UAgentPathFollowingComponent::UAgentPathFollowingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAgentPathFollowingComponent::BeginPlay()
{
	Super::BeginPlay();

  AActor* owner = GetOwner();
  m_followPath = true;
  if (owner)
  {
    UActorComponent* component = owner->GetComponentByClass(UPathfindingAgentComponent::StaticClass());
    if (component)
    {
      m_agentComponent = Cast<UPathfindingAgentComponent>(component);
      if (!m_agentComponent)
      {
        UE_LOG(LogTemp, Error, TEXT("Could not find a UPathfindingAgentComponent [PathFollwoingActorComponent][20]"));
      }
    }
  }
	
}

// Called every frame
void UAgentPathFollowingComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
  
  if (m_path.path.Num() > 0)
  {
    FVector currentNode = m_path.path.Last();

    if (m_shouldRotateTowardsNode)
      TurnTowardsClosestNode(DeltaTime, currentNode);

    MoveToClosestNode(DeltaTime, currentNode);
    ValidateDistanceToNode(currentNode, &m_path);

    //test if we're done with the path
    if (m_path.path.Num() == 0)
    {
      if (m_agentComponent || FindAgentComponent())
      {
        m_agentComponent->SetPath(nullptr);
      }
    }
  }
  else if (m_agentComponent || FindAgentComponent())
  {
    FPath* activePath = m_agentComponent->GetPath();
    if (activePath && activePath->path.Num() > 0)
      m_path = *activePath;
  }
}

void UAgentPathFollowingComponent::SetPath(TArray<FVector> a_path)
{
  m_path.path.Empty();
  for (int i = a_path.Num() - 1; i >= 0; i--)
  {
    m_path.path.Add(a_path[i]);
  }
}

void UAgentPathFollowingComponent::CancelPath()
{
  m_path.path.Empty();
}

bool UAgentPathFollowingComponent::FindAgentComponent()
{
  if (GetOwner())
  {
    UActorComponent* component = GetOwner()->GetComponentByClass(UPathfindingAgentComponent::StaticClass());
    if (component)
    {
      m_agentComponent = Cast<UPathfindingAgentComponent>(component);
      if (!m_agentComponent)
      {
        return false;
      }
    }
  }
  return true;
}

void UAgentPathFollowingComponent::MoveToClosestNode(float a_dt, FVector& a_closestNode)
{
  FTransform actorTransform = GetOwner()->GetTransform();
  FVector actorPos = actorTransform.GetTranslation();
  FVector direction = a_closestNode - GetOwner()->GetActorLocation();
  direction.Z = 0;
  direction.Normalize();

  actorPos += direction * (m_moveSpeed * a_dt);
  actorTransform.SetTranslation(actorPos);
  GetOwner()->SetActorTransform(actorTransform);
}

void UAgentPathFollowingComponent::TurnTowardsClosestNode(float a_dt, FVector& a_closestNode)
{
  FTransform ownerTransform = GetOwner()->GetTransform();
  FVector difference;

  difference = a_closestNode - ownerTransform.GetTranslation();
  FRotator rotator = FRotationMatrix::MakeFromX(difference).Rotator() - m_rotationOffset;
  FVector targetRotation = rotator.Euler();
  FVector myRotation = ownerTransform.Rotator().Euler();

  // make sure both rotations are positive when lerping, to prevent infinite spinning
  if (targetRotation.Z < 0)
    targetRotation.Z += 360;
  if (myRotation.Z < 0)
    myRotation.Z += 360;

  myRotation = FMath::VInterpTo(myRotation, targetRotation, a_dt, m_rotationSpeed);
  FRotator newRotator = FRotator::MakeFromEuler(myRotation);
  newRotator.Pitch = ownerTransform.Rotator().Pitch;
  newRotator.Roll = ownerTransform.Rotator().Roll;
  ownerTransform.SetRotation(newRotator.Quaternion());

  GetOwner()->SetActorTransform(ownerTransform);
}

void UAgentPathFollowingComponent::ValidateDistanceToNode(FVector& a_closestNode, FPath* a_path)
{
  AActor* owner = GetOwner();
  FVector myPosition = owner->GetTransform().GetTranslation();
  myPosition.Z = 0;
  FVector nodePos = a_closestNode;
  nodePos.Z = 0;
  float distance = FVector::Dist(myPosition, nodePos);
  if (distance <= m_distanceToRemoveNode)
    RemoveClosestNode(a_path);
}

void UAgentPathFollowingComponent::RemoveClosestNode(FPath* a_activePath)
{
  if (a_activePath->path.Num() > 0)
    a_activePath->path.RemoveAt(a_activePath->path.Num() - 1);
}
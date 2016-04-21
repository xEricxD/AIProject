// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/ActorComponent.h"
#include "PathfindingAgentComponent.h"
#include "AgentPathFollowingComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class FEATUREPROJECT_API UAgentPathFollowingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAgentPathFollowingComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;

  void FollowPath() { m_followPath = true; }
  void StopFollowingPath() { m_followPath = false; }

  UFUNCTION(Category = "Path follower", BlueprintCallable)
    void SetRotateTowardsNode(bool a_value) { m_shouldRotateTowardsNode = a_value; }
  UFUNCTION(Category = "Path follower", BlueprintCallable)
    void SetPath(TArray<FVector> a_path);
  UFUNCTION(Category = "Path follower", BlueprintCallable)
    bool IsTraversingPath() { return (m_path.path.Num() > 0); }

  void CancelPath();

private:
  UPROPERTY(EditAnywhere, Category = PathFollowingVariables)
    float m_distanceToRemoveNode;
  UPROPERTY(EditAnywhere, Category = PathFollowingVariables)
    float m_rotationSpeed;
  UPROPERTY(EditAnywhere, Category = PathFollowingVariables)
    float m_moveSpeed;
  UPROPERTY(EditAnywhere, Category = PathFollowingVariables)
    FRotator m_rotationOffset;
  UPROPERTY(EditAnywhere, Category = PathFollowingVariables)
    bool m_shouldRotateTowardsNode;

  bool m_followPath;

  bool FindAgentComponent();
  void MoveToClosestNode(float a_dt, FVector& a_closestNode);
  void TurnTowardsClosestNode(float a_dt, FVector& a_closestNode);
  void ValidateDistanceToNode(FVector& a_closestNode, FPath* a_path);
  void RemoveClosestNode(FPath* a_activePath);
  UPathfindingAgentComponent* m_agentComponent;

  FVector m_previousNode;
  FPath m_path;	
};

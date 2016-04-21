// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "JPSPlusGridActor.h"
#include "JPSPlusPathfinderActor.generated.h"

struct FPath;

UENUM(BlueprintType)
enum class EJumpDirections : uint8
{
  UP_LEFT = 0,
  UP,
  UP_RIGHT,
  LEFT,
  RIGHT,
  DOWN_LEFT,
  DOWN,
  DOWN_RIGHT
};

UCLASS()
class FEATUREPROJECT_API AJPSPlusPathfinderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AJPSPlusPathfinderActor();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  //a pathfinding agent can request a path, which this class will return when done
  void RequestPath(UActorComponent* a_requester, FPathfindingCell* a_startCell, FPathfindingCell* a_targetCell);

private:
  void DebugDraw();

  void AddStartCell(FJPSPlusPathfindingPacket &a_packet);
  bool SearchLoop(FJPSPlusPathfindingPacket &a_packet);

  void CreateDirectionJumpMap();
  void CreateDirectionDistanceMap();
  void CreateJumpDirectionMap();

  void AddNeighbours(FPathfindingCell* a_cell);
  void AddCellToOpenList(FPathfindingCell* a_newCell, FPathfindingCell* a_parentCell, short a_g);

  int GetHeuristic(FPathfindingCell* a_cell);
  void CreatePath(FPathfindingCell* a_cell, FPath* a_path);
  void Clear();

  //###################################
  //jump direction functions
  void JumpUpLeft(FPathfindingCell* a_current, short jumpDistance);
  void JumpUp(FPathfindingCell* a_current, short jumpDistance);
  void JumpUpRight(FPathfindingCell* a_current, short jumpDistance);
  void JumpLeft(FPathfindingCell* a_current, short jumpDistance);
  void JumpRight(FPathfindingCell* a_current, short jumpDistance);
  void JumpDownLeft(FPathfindingCell* a_current, short jumpDistance);
  void JumpDown(FPathfindingCell* a_current, short jumpDistance);
  void JumpDownRight(FPathfindingCell* a_current, short jumpDistance);
  //###################################

  //#########################
  //debug variables
  UPROPERTY(EditAnywhere)
    bool m_updateStepBased;
  UPROPERTY(EditAnywhere)
    bool m_debugStep;
  bool m_isFindingPath;
  //#########################

  UPROPERTY(EditAnywhere)
    uint16 m_maxSpinCount;

  AJPSPlusGridActor* m_grid;

  TQueue<FJPSPlusPathfindingPacket> m_pathQueue;
  TArray<FPathfindingCell*> m_fastStack;
  TArray<FPathfindingCell*> m_openList;
  TArray<FPathfindingCell*> m_closedList;

  FPathfindingCell* m_goalCell;

  //map of jumps and vector representing the jump
  TMap < EJumpDirections, FVector > m_jumpDirectionMap;
  //map of directions and corresponding jumps
  TMap < FVector, TArray< EJumpDirections > > m_directionJumpMap;
  //map of directions and corresponding array indeces for jump distances
  TMap < FVector, short > m_directionDistanceMap;

  // function pointer that returns void type and takes in 2 arguments: a pathfindingcell and a short
  typedef void (AJPSPlusPathfinderActor::*FunctionPtrType)(FPathfindingCell* a_current, short jumpDistance);
  // map of function pointers with jump directions
  TMap < EJumpDirections, FunctionPtrType > m_jumpMap;	
};

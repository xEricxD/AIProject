//Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/Actor.h"
#include "JPSGridActor.h"
#include "JPSPathfinder.generated.h"

struct FPath;
UCLASS()
class FEATUREPROJECT_API AJPSPathfinderActor : public AActor
{
	GENERATED_BODY()
	
public:	
	//Sets default values for this actor's properties
  AJPSPathfinderActor();

	//Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	//Called every frame
	virtual void Tick( float DeltaSeconds ) override;

  void RequestPath(UActorComponent* a_requester, FJPSCell* a_startCell, FJPSCell* a_targetCell);

private:
  void DrawDebug();
  void AddStartCell(FJPSPathfindingPacket &a_packet);
  bool SearchLoop(FJPSPathfindingPacket &a_packet);

  //JPS functions
  void AddNeighbors(FJPSCell* a_cell);
  short GetGValueJPS(FJPSCell* a_jumpPoint, FJPSCell* a_parent);
  FJPSCell* FindJumpPoint(FJPSCell* a_cell, FVector a_dir, FJPSCell* a_startcell, FJPSCell* a_targetCell);
  bool CheckStraightForcedNeighbor(FJPSCell* a_cell, FVector a_offset, FVector a_dir);
  bool CheckDiagonalForcedNeighbor(FJPSCell* a_cell, FVector a_offset, FVector a_dir);

  TArray<FJPSCell*> PruneStraight(FJPSCell* a_current, FVector a_jumpDirection);
  TArray<FJPSCell*> PruneDiagonal(FJPSCell* a_current, FVector a_jumpDirection);

  int GetHeuristic(FJPSCell* a_cell);
  void CreatePath(FJPSCell* a_cell, FPath* a_path);
  void Clear();

  UPROPERTY(EditAnywhere)
    uint16 m_maxSpinCount;

  UPROPERTY(EditAnywhere)
    bool m_updateStepBased;
  UPROPERTY(EditAnywhere)
    bool m_debugStep;
  bool m_isFindingPath;

  UPROPERTY(EditAnywhere)
    bool m_debugdraw;

  AJPSGridActor* m_grid;

  TQueue<FJPSPathfindingPacket> m_pathQueue;

  TArray<FJPSCell*> m_openList;
  TArray<FJPSCell*> m_closedList;

  FJPSCell* m_goalCell;
};
